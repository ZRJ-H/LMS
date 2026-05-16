const fs = require('fs');
const path = require('path');

const PATHS = {
    KB_FILE: path.join(process.cwd(), 'docs/knowledge_base.md'),
    KB_INDEX: path.join(process.cwd(), 'docs/kb_index.json'),
    TODO_FILE: path.join(process.cwd(), 'docs/todo_changes.md'),
    HANDOFF_FILE: path.join(process.cwd(), 'handoff.md')
};

function saveKnowledge({ title, block, aha, hood, code }) {
    let index = [];
    if (fs.existsSync(PATHS.KB_INDEX)) {
        try {
            index = JSON.parse(fs.readFileSync(PATHS.KB_INDEX, 'utf-8'));
        } catch (e) {
            index = [];
        }
    }

    if (index.includes(title)) {
        return { success: false, message: `知识点 [${title}] 已存在于索引中，请提醒用户去复习，不要重复写入。` };
    }

    const date = new Date().toISOString().split('T')[0];
    const content = `\n### 📝 [${title}] (日期: ${date})\n- **我的困惑 (The Block)**：${block}\n- **破局思路 (The Aha Moment)**：${aha}\n- **技术真相 (Under the Hood)**：${hood}\n- **代码快照 (Code Snippet)**：\n\`\`\`\n${code}\n\`\`\`\n---\n`;

    fs.appendFileSync(PATHS.KB_FILE, content, 'utf-8');
    index.push(title);
    fs.writeFileSync(PATHS.KB_INDEX, JSON.stringify(index, null, 2), 'utf-8');

    return { success: true, message: "知识库已成功追加，索引已更新。" };
}

function saveTodo({ filePath, goal }) {
    const content = `\n- [ ] **文件路径**：${filePath}\n  **修改目标**：${goal}\n`;
    fs.appendFileSync(PATHS.TODO_FILE, content, 'utf-8');
    return { success: true, message: "待办事项已添加。" };
}

function saveHandoff({ goal, files, changes, deadends, nextSteps, notes }) {
    const now = new Date();
    const ts = now.toISOString().replace('T', ' ').substring(0, 19);
    const fileList = (files || []).map(f => `- ${f}`).join('\n');
    const changeList = (changes || []).map(c => `- ${c}`).join('\n');
    const deadendList = (deadends || []).map(d => `- ${d}`).join('\n');
    const nextList = (nextSteps || []).map((s, i) => `${i + 1}. ${s}`).join('\n');
    const notesBlock = notes ? `\n## 💡 Notes\n${notes}\n` : '';

    const content = `# Handoff — ${ts}

## 🎯 Goal
${goal || '(待填写)'}

## 📁 Current Files
${fileList || '(待填写)'}

## ✅ Changes Made
${changeList || '(暂无)'}

## ❌ Dead Ends
${deadendList || '(暂无)'}

## 🔜 Next Steps
${nextList || '(待填写)'}
${notesBlock}
`;

    fs.writeFileSync(PATHS.HANDOFF_FILE, content, 'utf-8');
    return { success: true, message: "handoff.md 已写入，可以安全结束本次对话。" };
}

/* ============================================================
 *      学习日志 (Learning Log) — 项目过程档案
 * ============================================================ */
function saveLearn({ title, date, tags, project, problem, rootCause, solution, learned, highlight, code, supplementalTo }) {
    const LEARN_FILE = path.join(process.cwd(), 'docs/learning_log.md');
    const LEARN_INDEX = path.join(process.cwd(), 'docs/learn_index.json');

    let index = [];
    if (fs.existsSync(LEARN_INDEX)) {
        try { index = JSON.parse(fs.readFileSync(LEARN_INDEX, 'utf-8')); }
        catch (e) { index = []; }
    }

    /* 如果是补充已有卡片 */
    if (supplementalTo) {
        const existing = index.find(e => e.title === supplementalTo);
        if (!existing) return { success: false, message: `目标卡片 [${supplementalTo}] 不存在，无法补充。` };
        const content = `\n### 🔗 补充 (${date})\n**问题**：${problem}\n**解决**：${solution}\n**学到**：${learned}\n**代码**：\n\`\`\`\n${code}\n\`\`\`\n---\n`;
        fs.appendFileSync(LEARN_FILE, content, 'utf-8');
        return { success: true, message: `已补充到卡片 [${supplementalTo}]` };
    }

    /* 查重 */
    if (index.some(e => e.title === title)) {
        return { success: false, message: `学习卡片 [${title}] 已存在，提醒用户去复习。如需补充请指定 supplementalTo。` };
    }

    const tagList = (tags || []).join(', ');
    const content = `\n---\ntitle: "${title}"\ndate: ${date}\ntags: [${tagList}]\nproject: ${project || 'LMS'}\n---\n\n## 问题\n${problem}\n\n## 根因\n${rootCause}\n\n## 解决\n${solution}\n\n## 学到\n${learned}\n\n## 亮点\n${highlight}\n\n## 代码\n\`\`\`c\n${code}\n\`\`\`\n`;

    fs.appendFileSync(LEARN_FILE, content, 'utf-8');
    index.push({ title, date, tags: tags || [], project: project || 'LMS' });
    fs.writeFileSync(LEARN_INDEX, JSON.stringify(index, null, 2), 'utf-8');

    /* 检查同 tag 相关卡片 */
    const related = index.filter(e =>
        e.title !== title &&
        e.tags && tags &&
        e.tags.some(t => tags.includes(t))
    );
    const relatedMsg = related.length > 0
        ? ` 相关卡片: ${related.map(r => `[${r.title}]`).join(', ')}`
        : '';

    return { success: true, message: `学习卡片 [${title}] 已记录。${relatedMsg}` };
}

const args = JSON.parse(process.argv[2]);
if (args.action === 'save_kb') {
    console.log(JSON.stringify(saveKnowledge(args.data)));
} else if (args.action === 'save_todo') {
    console.log(JSON.stringify(saveTodo(args.data)));
} else if (args.action === 'save_handoff') {
    console.log(JSON.stringify(saveHandoff(args.data)));
} else if (args.action === 'save_learn') {
    console.log(JSON.stringify(saveLearn(args.data)));
}
