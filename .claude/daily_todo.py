#!/usr/bin/env python3
r"""
每日进度追踪工具 (Daily Progress Tracker)

Usage:
  python .claude/daily_todo.py           显示今日计划 + 指标仪表盘
  python .claude/daily_todo.py done      标记今天完成（扫描 C_LMS 验证）
  python .claude/daily_todo.py check     仅扫描 C_LMS，不标记
  python .claude/daily_todo.py snippet   生成今日日报摘要（用于粘贴到 WPS 日报）

双文件夹工作流：
  D:/MyProject/LMS     — Vibe coding 区（计划源 plan.md）
  D:/MyProject/C_LMS   — 交付区（实际代码文件 = 完成的唯一证据）
"""

import json
import os
import re
import sys
from datetime import date, datetime, timedelta
from pathlib import Path

# --- 路径配置 -----------------------------------------------
LMS_DIR = Path(r"D:\MyProject\LMS")
C_LMS_DIR = Path(r"D:\MyProject\C_LMS")
PLAN_FILE = LMS_DIR / "plan.md"
PROGRESS_FILE = LMS_DIR / "docs" / "progress.json"

# --- 工具函数 -----------------------------------------------

def scan_c_lms():
    """扫描 C_LMS 目录，返回所有 .c/.h 文件及其修改时间"""
    files = {}
    if not C_LMS_DIR.exists():
        return files
    for ext in ("*.c", "*.h"):
        for fp in C_LMS_DIR.glob(ext):
            mtime = os.path.getmtime(fp)
            files[fp.name] = datetime.fromtimestamp(mtime).strftime("%Y-%m-%d %H:%M")
    return files


def load_progress():
    """加载进度文件，不存在则返回默认结构"""
    if PROGRESS_FILE.exists():
        try:
            with open(PROGRESS_FILE, "r", encoding="utf-8") as f:
                data = json.load(f)
                if "days" not in data:
                    data["days"] = {}
                if "c_lms_snapshot" not in data:
                    data["c_lms_snapshot"] = {}
                return data
        except (json.JSONDecodeError, IOError):
            pass
    return {"days": {}, "c_lms_snapshot": {}}


def save_progress(data):
    """保存进度文件"""
    PROGRESS_FILE.parent.mkdir(parents=True, exist_ok=True)
    with open(PROGRESS_FILE, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=2)


def parse_plan():
    """
    解析 plan.md，返回按日期组织的任务列表。
    每个条目: {date, day_label, module, goal, subtasks, expected_files}
    """
    if not PLAN_FILE.exists():
        return []

    with open(PLAN_FILE, "r", encoding="utf-8") as f:
        lines = f.readlines()

    entries = []
    current_module = ""
    current_module_start = None
    current_module_end = None
    current_day_label = ""
    current_date = None
    current_goal = ""
    current_subtasks = []
    in_module = False
    collect_mode = False  # True when we're inside a day entry collecting content
    module_has_subdays = False

    for line in lines:
        # -- Module heading: ## N. 模块名 D1（YYYY-MM-DD）or range --
        m = re.match(r"^##\s+\d+\.\s+(.+?)[（(](\d{4}-\d{2}-\d{2})\s*(?:[~～]\s*(\d{4}-\d{2}-\d{2}))?[）)]", line)
        if m:
            # Flush any pending entry from previous module/day
            if current_date and collect_mode:
                entries.append({
                    "date": current_date,
                    "day_label": current_day_label or "Day 1",
                    "module": current_module,
                    "goal": current_goal,
                    "subtasks": current_subtasks,
                    "expected_files": _extract_files(current_goal, current_subtasks)
                })

            current_module = m.group(1).strip()
            current_module_start = m.group(2)
            current_module_end = m.group(3) if m.group(3) else m.group(2)
            current_date = current_module_start
            current_day_label = ""
            current_goal = ""
            current_subtasks = []
            in_module = True
            module_has_subdays = False
            # Only collect at module level if single date (no sub-days)
            has_range = bool(m.group(3))
            collect_mode = not has_range
            continue

        # -- Day sub-heading: ### Day N（MM-DD）or range --
        m = re.match(r"^###\s+(Day\s*\d+)\s*[（(](\d{2}-\d{2})\s*(?:[~～]\s*(\d{2}-\d{2}))?[）)]", line)
        if m and in_module:
            module_has_subdays = True

            # Flush previous day entry
            if current_date and collect_mode:
                entries.append({
                    "date": current_date,
                    "day_label": current_day_label,
                    "module": current_module,
                    "goal": current_goal,
                    "subtasks": current_subtasks,
                    "expected_files": _extract_files(current_goal, current_subtasks)
                })

            current_day_label = m.group(1)
            day_month_day = m.group(2)  # e.g., "05-14"
            end_mm_dd = m.group(3)  # e.g., "05-16" for ranges

            # Infer year from module's start date
            year = current_module_start[:4]
            current_date = f"{year}-{day_month_day}"

            current_goal = ""
            current_subtasks = []
            collect_mode = True
            continue

        # -- Separator line: skip --
        if line.strip() == "---":
            continue

        # -- Goal line: **目标**：... --
        if collect_mode and "**目标**" in line:
            current_goal = line.split("**目标**", 1)[-1].strip().lstrip("：:").strip()
            continue

        # -- Subtask line: - ... --
        if collect_mode and re.match(r"^-\s+", line):
            task = re.sub(r"^-\s+", "", line).strip()
            if task:
                current_subtasks.append(task)
            continue

    # Flush last entry
    if current_date and collect_mode:
        entries.append({
            "date": current_date,
            "day_label": current_day_label,
            "module": current_module,
            "goal": current_goal,
            "subtasks": current_subtasks,
            "expected_files": _extract_files(current_goal, current_subtasks)
        })

    return entries


def _extract_files(goal, subtasks):
    """从目标和子任务文本中提取预期产出的文件名 (.c/.h)"""
    text = goal + " " + " ".join(subtasks)
    # Match backtick-enclosed .c/.h references
    files = re.findall(r"`([^`]+\.(?:c|h))`", text)
    # Also match unquoted references like "dao_user.c、svc_system.c"
    files += re.findall(r"(?<![`\w])(\w+\.(?:c|h))", text)
    return sorted(set(f for f in files if f not in ("common.h", "common.c") or True))


def get_today_plan(entries):
    """返回今天对应的计划条目列表"""
    today_obj = date.today()
    today = today_obj.isoformat()
    return [e for e in entries if e["date"] == today]


def get_overdue(entries, progress):
    """返回今天之前未完成的条目"""
    today_obj = date.today()
    today = today_obj.isoformat()
    overdue = []
    for e in entries:
        if e["date"] >= today:
            continue
        day_status = progress.get("days", {}).get(e["date"], {})
        if day_status.get("status") != "done":
            # Check if C_LMS has all expected files for this day
            expected = e.get("expected_files", [])
            c_lms_files = scan_c_lms()
            verified = [f for f in expected if f in c_lms_files]
            overdue.append({**e, "verified_files": verified,
                           "all_verified": len(expected) > 0 and len(verified) == len(expected)})
    return overdue


def get_completed(entries, progress):
    """返回已完成的条目"""
    completed = []
    for e in entries:
        day_status = progress.get("days", {}).get(e["date"], {})
        if day_status.get("status") == "done":
            completed.append(e)
    return completed


def get_all_dates_in_range(start_str, end_str):
    """生成日期范围内的所有日期"""
    start = datetime.strptime(start_str, "%Y-%m-%d").date()
    end = datetime.strptime(end_str, "%Y-%m-%d").date()
    dates = []
    current = start
    while current <= end:
        dates.append(current.isoformat())
        current += timedelta(days=1)
    return dates


def compute_metrics(entries, progress):
    """计算指标仪表盘数据"""
    today_obj = date.today()
    today = today_obj.isoformat()
    all_plan_dates = sorted(set(e["date"] for e in entries))

    # 已完成天数
    done_dates = set()
    for d, s in progress.get("days", {}).items():
        if s.get("status") == "done":
            done_dates.add(d)

    # 模块进度：C_LMS 有对应预期文件的模块算完成
    c_lms_files = scan_c_lms()
    modules_done = set()
    for e in entries:
        expected = e.get("expected_files", [])
        if expected:
            verified = [f for f in expected if f in c_lms_files]
            if len(verified) == len(expected):
                modules_done.add(e["module"])

    # 总模块数（去重）
    all_modules = sorted(set(e["module"] for e in entries if e["module"]))

    # 当前应该做到哪一天
    plan_dates_before_today = [d for d in all_plan_dates if d <= today]
    expected_done = len(set(plan_dates_before_today))
    actual_done = len([d for d in done_dates if d <= today])

    # 滞后天数
    lag_days = 0
    if plan_dates_before_today:
        last_plan_date = max(plan_dates_before_today)
        last_done_date = max(d for d in done_dates if d <= today) if done_dates else None
        if last_done_date:
            from datetime import date as dt_date
            lag_days = (dt_date.fromisoformat(last_plan_date) - dt_date.fromisoformat(last_done_date)).days
        else:
            lag_days = expected_done

    # 连续打卡
    streak = 0
    check_date = today_obj - timedelta(days=1)
    while check_date.isoformat() in done_dates:
        streak += 1
        check_date -= timedelta(days=1)
    # Check if today is also done
    if today in done_dates:
        streak += 1

    return {
        "total_modules": len(all_modules),
        "modules_done": len(modules_done),
        "total_plan_days": len(set(all_plan_dates)),
        "actual_done": actual_done,
        "expected_done": expected_done,
        "lag_days": lag_days,
        "streak": streak,
        "done_dates": done_dates,
    }


def format_bar(value, maximum, width=10):
    """绘制进度条"""
    filled = int(value / max(maximum, 1) * width)
    return "#" * filled + "-" * (width - filled)


# --- 视图渲染 ----------------------------------------------

def show_dashboard(entries, progress):
    """输出完整日报仪表盘"""
    today = date.today()
    today_str = today.isoformat()
    metrics = compute_metrics(entries, progress)
    c_lms_files = scan_c_lms()

    print()
    print("+======================================================+")
    print(f"|  [DATE] {today_str}  项目日报                               |")
    print("+======================================================+")

    # -- 今日任务 --
    today_entries = get_today_plan(entries)
    if today_entries:
        print("|                                                      |")
        print("|  [>>] 今日任务                                        |")
        for e in today_entries:
            module_short = e["module"][:30] if e["module"] else ""
            print(f"|  [ ] [{e['day_label']}] {module_short}")
            print(f"|     {e['goal'][:50]}")
            expected = e.get("expected_files", [])
            if expected:
                print(f"|     预期产出: {', '.join(expected[:5])}")
        print("|                                                      |")
    else:
        print("|                                                      |")
        print("|  [>>] 今日无计划任务，可以查漏补缺或提前推进               |")
        print("|                                                      |")

    # -- 逾期未完成 --
    overdue = get_overdue(entries, progress)
    if overdue:
        print("|  [!]  逾期未完成                                       |")
        for e in overdue[:5]:
            module_short = e["module"][:25] if e["module"] else ""
            verified_str = ""
            if e.get("verified_files"):
                verified_str = f" [C_LMS有: {', '.join(e['verified_files'][:3])}]"
            auto_done = " [v]" if e.get("all_verified") else ""
            print(f"|  [ ] {e['date']} [{e['day_label']}] {module_short}{auto_done}")
            print(f"|     目标: {e['goal'][:45]}{verified_str}")
        if len(overdue) > 5:
            print(f"|     ... 还有 {len(overdue) - 5} 项逾期")
        print("|                                                      |")

    # -- 已完成 --
    completed = get_completed(entries, progress)
    if completed:
        print("|  [OK] 已完成                                           |")
        for e in completed[-10:]:
            module_short = e["module"][:30] if e["module"] else ""
            print(f"|  [v] {e['date']} [{e['day_label']}] {module_short}")
        print("|                                                      |")

    # -- 指标仪表盘 --
    print("|  ------------ 指标仪表盘 ------------                 |")
    bar = format_bar(metrics["modules_done"], metrics["total_modules"])
    print(f"|  [PKG] 模块进度  {bar} {metrics['modules_done']}/{metrics['total_modules']}")
    bar2 = format_bar(metrics["actual_done"], metrics["total_plan_days"])
    print(f"|  [OK] 完成率    {bar2} {metrics['actual_done']}/{metrics['total_plan_days']} ({int(metrics['actual_done']/max(metrics['total_plan_days'],1)*100)}%)")
    print(f"|  [FOCUS] 连续打卡  {metrics['streak']} 天")
    lag = metrics["lag_days"]
    if lag <= 0:
        print(f"|  [TIME] 进度偏差  超前 {abs(lag)} 天")
    elif lag <= 2:
        print(f"|  [TIME] 进度偏差  滞后 {lag} 天 [!]")
    else:
        print(f"|  [TIME] 进度偏差  滞后 {lag} 天 [ALERT] 需要加速！")
    print("|                                                      |")

    # -- C_LMS 文件摘要 --
    if c_lms_files:
        print("|  [DIR] C_LMS 现有文件 ({})                              |".format(len(c_lms_files)))
        for fname in sorted(c_lms_files.keys())[:8]:
            print(f"|     {fname}")
        if len(c_lms_files) > 8:
            print(f"|     ... 还有 {len(c_lms_files) - 8} 个文件")
        print("|                                                      |")

    print("+======================================================+")
    print()
    print("  提示: python .claude/daily_todo.py done  → 标记今日完成")
    print("        python .claude/daily_todo.py check → 扫描 C_LMS 对照")
    print()


# --- 动作函数 ----------------------------------------------

def do_done():
    """标记今天完成"""
    entries = parse_plan()
    if not entries:
        print("[ERR] 未找到计划文件 plan.md")
        return

    progress = load_progress()
    today_obj = date.today()
    today = today_obj.isoformat()

    # 扫描 C_LMS
    c_lms_files = scan_c_lms()
    old_snapshot = progress.get("c_lms_snapshot", {})
    new_files = {k: v for k, v in c_lms_files.items() if k not in old_snapshot}

    # 尝试匹配今天计划中的预期文件
    today_entries = get_today_plan(entries)
    verified_files = []
    auto_matched = False

    if today_entries:
        for e in today_entries:
            expected = e.get("expected_files", [])
            for f in expected:
                if f in c_lms_files:
                    verified_files.append(f)

    # 也检查是否逾期条目现在有了产出
    overdue = get_overdue(entries, progress)
    for e in overdue:
        expected = e.get("expected_files", [])
        for f in expected:
            if f in c_lms_files and f not in verified_files:
                verified_files.append(f)
        if e.get("all_verified"):
            # 自动标记逾期的为完成
            progress["days"][e["date"]] = {
                "status": "done",
                "verified_files": [f for f in expected if f in c_lms_files],
                "note": "自动检测(逾期补完)"
            }
            print(f"[OK] 自动标记: {e['date']} [{e['day_label']}] 预期文件已全部到位")

    # 更新今天的状态
    if verified_files:
        progress["days"][today] = {
            "status": "done",
            "verified_files": verified_files,
            "note": ""
        }
        auto_matched = True
    else:
        # 没有匹配到文件但用户说做完了 — 可能产出不是 .c/.h
        if new_files:
            progress["days"][today] = {
                "status": "done",
                "verified_files": list(new_files.keys()),
                "note": "根据新增文件自动标记"
            }
            auto_matched = True
        else:
            progress["days"][today] = {
                "status": "done",
                "verified_files": [],
                "note": "手动确认(未检测到新文件)"
            }

    # 更新快照
    progress["c_lms_snapshot"] = c_lms_files
    save_progress(progress)

    print()
    print(f"[OK] 已标记 {today} 为完成")
    if verified_files:
        print(f"   C_LMS 验证文件: {', '.join(verified_files)}")
    if new_files and not verified_files:
        print(f"   C_LMS 新增文件: {', '.join(list(new_files.keys())[:10])}")
    if not c_lms_files:
        print("   [!]  C_LMS 目录未发现 .c/.h 文件，请确认迁移状态")
    print()

    # 显示更新后的仪表盘
    show_dashboard(entries, progress)

    # 生成日报摘要
    print("-" * 55)
    print(snippet_today(entries, progress))
    print("-" * 55)
    print("  [LIST] 上方是今日日报摘要，可复制到 WPS 日报文档中")
    print()


def do_check():
    """扫描 C_LMS 并与计划对照"""
    entries = parse_plan()
    if not entries:
        print("[ERR] 未找到计划文件 plan.md")
        return

    progress = load_progress()
    c_lms_files = scan_c_lms()

    print()
    print("=" * 55)
    print("  C_LMS 文件 <=> 计划对照")
    print("=" * 55)

    if not c_lms_files:
        print("  [!]  C_LMS 目录未发现 .c/.h 文件")
        print()
        return

    print(f"\n  [DIR] C_LMS 现有文件 ({len(c_lms_files)} 个):")
    for fname, mtime in sorted(c_lms_files.items()):
        print(f"     {fname:30s}  ({mtime})")
    print()

    # 对照每个计划日期的预期文件
    print("  [LIST] 计划对照表:")
    print(f"  {'日期':<12} {'Day':<8} {'状态':<8} {'预期文件':<40} {'C_LMS中'}")
    print("  " + "-" * 80)

    matched = 0
    total = 0
    for e in entries:
        expected = e.get("expected_files", [])
        if not expected:
            continue
        total += 1
        verified = [f for f in expected if f in c_lms_files]
        all_ok = len(verified) == len(expected)
        if all_ok:
            matched += 1
        status = "[v] 完成" if all_ok else f"△ {len(verified)}/{len(expected)}"
        print(f"  {e['date']:<12} {e['day_label']:<8} {status:<8} {', '.join(expected):<40} {', '.join(verified) if verified else '(无)'}")

    if total > 0:
        print(f"\n  对照结果: {matched}/{total} 天预期文件全部到位")
    else:
        print("\n  (计划中未标注具体文件名，无法自动对照)")
    print()


def snippet_today(entries, progress):
    """生成今日日报文本摘要，供粘贴到 WPS 日报 doc"""
    today = date.today()
    today_str = today.isoformat()
    today_entries = get_today_plan(entries)
    yesterday_entries = [e for e in entries if e["date"] == (today - timedelta(days=1)).isoformat()]

    lines = []
    lines.append(f"日期：{today_str}")
    lines.append("")

    # 规划内容
    lines.append("【规划内容 — 今天应完成】")
    if today_entries:
        for e in today_entries:
            lines.append(f"  {e['goal']}")
            for s in e.get("subtasks", [])[:5]:
                lines.append(f"    - {s}")
    else:
        lines.append("  (无特定计划，查漏补缺或提前推进)")
    lines.append("")

    # 实际完成
    lines.append("【完成情况】")
    day_status = progress.get("days", {}).get(today_str, {})
    if day_status.get("status") == "done":
        vf = day_status.get("verified_files", [])
        if vf:
            lines.append(f"  已完成，C_LMS 产出文件: {', '.join(vf)}")
        else:
            lines.append("  已完成（已手动确认）")
    else:
        lines.append("  (待填写)")
    lines.append("")

    # 昨日完成
    if yesterday_entries:
        lines.append("【昨日（{}) 回顾】".format((today - timedelta(days=1)).isoformat()))
        y_status = progress.get("days", {}).get((today - timedelta(days=1)).isoformat(), {})
        if y_status.get("status") == "done":
            lines.append(f"  已完成。产出: {', '.join(y_status.get('verified_files', ['无记录']))}")
        else:
            lines.append("  (未标记完成)")
        lines.append("")

    lines.append("【存在问题】")
    lines.append("  (待填写)")
    lines.append("")
    lines.append("【工作感想】")
    lines.append("  (待填写)")

    return "\n".join(lines)


# --- 入口 --------------------------------------------------

def main():
    entries = parse_plan()

    if not entries:
        print("[ERR] 未找到 plan.md 或解析失败")
        return

    progress = load_progress()

    cmd = sys.argv[1] if len(sys.argv) > 1 else ""

    if cmd == "done":
        do_done()
    elif cmd == "check":
        do_check()
    elif cmd == "snippet":
        print(snippet_today(entries, progress))
    else:
        show_dashboard(entries, progress)


if __name__ == "__main__":
    main()
