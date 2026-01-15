---
description: Execute task with continual learning - read lessons, perform task, reflect, and update knowledge base
agent: Evolve
model: opencode/minimax-m2.1-free
---

# Smart Task with Continual Learning (OpenCode MiniMax M2.1 Free Powered)

You are executing a task with Continual Learning workflow enabled, powered by OpenCode's free MiniMax M2.1 model for efficient coding and agentic workflows. This ensures you learn from each task and improve future performance.

## 📖 Phase 1: Context Loading (MANDATORY - DO THIS FIRST)

Before generating any code or making any changes, you MUST:

1. **Read lessons file**: Use `read` tool to read `.opencode/memory/lessons_learned.md`
2. **Analyze lessons**: Look for:
   - Related Do's that should guide your approach
   - Related Don'ts that you must avoid
   - Edge Cases relevant to the task
   - Project-specific patterns to follow
3. **Acknowledge learning**: Briefly summarize what lessons are relevant to this task

⚠️ **DO NOT proceed to execution until you have read and acknowledged lessons.**

---

## ⚙️ Phase 2: Task Execution (MiniMax M2.1 Optimized)

Execute the user's requested task, keeping lessons in mind:

1. Apply relevant Do's and patterns
2. Avoid documented Don'ts and anti-patterns
3. Consider documented Edge Cases
4. Follow Project-Specific Patterns

**MiniMax M2.1 Advantages:**
- Excels in code generation and multi-file editing
- Strong performance on compile-run-fix loops
- Test-validated repair capabilities
- Efficient for long-horizon planning
- Fast perceived response times
- Leading multilingual coding performance

If you discover a new pattern, insight, or make a mistake during execution, take note of it for the reflection phase.

---

## 🤔 Phase 3: Retrospective & Reflection (MANDATORY)

After completing the task, you MUST reflect on your experience:

1. **Was the task successful?**
   - Did everything work as expected?
   - Were there any issues or errors?

2. **Did you learn anything new?**
   - Did you discover a new pattern or approach?
   - Did you encounter an edge case not documented?

3. **Did lessons help?**
   - Did the knowledge base help you avoid a mistake?
   - Were any lessons particularly relevant?

4. **What should be documented?**
   - Any new patterns discovered?
   - Any mistakes made that others should avoid?
   - Any edge cases encountered?

---

## 📝 Phase 4: Update Knowledge Base (If Applicable)

If ANY of the following occurred, you MUST update `.opencode/memory/lessons_learned.md`:

### When to Update:
- ✅ You discovered a new best practice (Add to "Do's")
- ✅ You made a mistake that should be avoided (Add to "Don'ts")
- ✅ You encountered a tricky edge case (Add to "Edge Cases & Gotchas")
- ✅ You learned about a project-specific pattern (Add to "Project-Specific Patterns")

### Format for New Entries:
Add entries under the appropriate section using this format:

```
### [Date]

**Issue**: [Brief description of what happened or was learned]

**Lesson**: [Concise explanation of what to do or avoid]

**Example**: [Optional - short code snippet or example]

**Impact**: [Optional - why this matters]
```

### How to Update:
1. Read the current `lessons_learned.md` file
2. Append your new entry to the appropriate section
3. Update the "Last Updated" date at the top
4. Use the `write` tool to save the updated file

---

## 🎯 OpenCode MiniMax M2.1 Free + Continual Learning Goal

Continual Learning with MiniMax M2.1 means:
- Every task makes you smarter
- Every mistake becomes a lesson
- Every success becomes a pattern
- MiniMax's efficient coding + learning loop = rapid iteration
- Future tasks benefit from past experience with better performance
- Knowledge is cumulative and improves over time

**Remember**: OpenCode's free MiniMax M2.1 excels at tool use and reasoning. Leverage its strengths for complex tasks while maintaining the continual learning workflow.

---

*Knowledge is cumulative. MiniMax + Learning = Better Code Faster!*