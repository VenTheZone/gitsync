---
name: skill-writer
description: Guide users through creating Agent Skills for OpenCode. Use when creating, writing, authoring, or designing Skills for OpenCode with best practices, validation, and subagent integration.
---

# Skill Writer

This Skill helps you create well-structured Agent Skills for OpenCode that follow best practices and validation requirements, with subagent integration capabilities.

## When to use this Skill

Use this Skill when:
- Creating a new OpenCode Skill
- Writing or updating SKILL.md files
- Designing skill structure and frontmatter
- Troubleshooting skill discovery issues
- Converting existing prompts or workflows into Skills
- Setting up skills that work with subagents

## Instructions

### Step 1: Determine Skill scope

First, understand what Skill should do:

1. **Ask clarifying questions**:
   - What specific capability should this Skill provide?
   - When should OpenCode use this Skill?
   - What tools or resources does it need?
   - Is this for personal use or team sharing?
   - Should this work with subagents?

2. **Keep it focused**: One Skill = one capability
   - Good: "PDF form filling", "Excel data analysis"
   - Too broad: "Document processing", "Data tools"

### Step 2: Choose Skill location

Determine where to create the Skill:

**OpenCode Skills** (`.opencode/skill/`):
- Individual workflows and preferences
- Project-specific expertise
- Team workflows and conventions
- Shared utilities (committed to git)
- Works with Evolve agent integration

**Subagent-enabled Skills**:
- When creating skills that need to trigger other agents
- Must reference subagents in frontmatter or configuration
- Should use OpenCode's agent management system

### Step 3: Create Skill structure

Create directory and files:

```bash
mkdir -p .opencode/skills/skill-name
```

For multi-file Skills:
```
skill-name/
├── SKILL.md (required)
├── reference.md (optional)
├── examples.md (optional)
├── scripts/
│   └── helper.py (optional)
└── templates/
    └── template.txt (optional)
```

### Step 4: Write SKILL.md frontmatter

Create YAML frontmatter with required fields:

```yaml
---
name: skill-name
description: Brief description of what this does and when to use it
agent: subagent-name  # Optional: for subagent-enabled skills
---
```

**Field requirements**:

- **name**:
  - Lowercase letters, numbers, hyphens only
  - Max 64 characters
  - Must match directory name
  - Good: `pdf-processor`, `git-commit-helper`
  - Bad: `PDF_Processor`, `Git Commits!`

- **description**:
  - Max 1024 characters
  - Include BOTH what it does AND when to use it
  - Use specific trigger words users would say
  - Mention file types, operations, and context

- **agent** (optional):
  - Name of subagent to trigger
  - Use when skill creates, modifies, or manages other agents
  - Must be available in `evolve agents list`

**Optional frontmatter fields**:

- **allowed-tools**: Restrict tool access (comma-separated list)
  ```yaml
  allowed-tools: Read, Grep, Glob, bash
  ```
  Use for:
  - Read-only Skills
  - Security-sensitive workflows
  - Limited-scope operations

- **requires-plugins**: List required OpenCode plugins
  ```yaml
  requires-plugins: continual-learning, mcp-server
  ```

- **version**: Skill version for compatibility tracking
  ```yaml
  version: 1.0.0
  ```

### Step 5: Write effective descriptions

The description is critical for OpenCode to discover your Skill.

**Formula**: `[What it does] + [When to use it] + [Key triggers]`

**Examples**:

✅ **Good**:
```yaml
description: Extract text and tables from PDF files, fill forms, merge documents. Use when working with PDF files or when user mentions PDFs, forms, or document extraction.
agent: file-processor  # Triggers file management subagent
```

✅ **Good**:
```yaml
description: Analyze Excel spreadsheets, create pivot tables, and generate charts. Use when working with Excel files, spreadsheets, or analyzing tabular data in .xlsx format.
agent: data-analyst  # Triggers data analysis subagent
```

❌ **Too vague**:
```yaml
description: Helps with documents
description: For data analysis
```

**Tips**:
- Include specific file extensions (.pdf, .xlsx, .json)
- Mention common user phrases ("analyze", "extract", "generate")
- List concrete operations (not generic verbs)
- Add context clues ("Use when...", "For...")

### Step 6: Structure Skill content

Use clear Markdown sections:

```markdown
# Skill Name

Brief overview of what this Skill does.

## Quick start

Provide a simple example to get started immediately.

## Instructions

Step-by-step guidance for OpenCode:
1. First step with clear action
2. Second step with expected outcome
3. Handle edge cases

## Examples

Show concrete usage examples with code or commands.

## Best practices

- Key conventions to follow
- Common pitfalls to avoid
- When to use vs. not use

## Requirements

List any dependencies or prerequisites:
```bash
pip install package-name
```

## Advanced usage

For complex scenarios, see [reference.md](reference.md).

## Subagent Integration

This skill integrates with OpenCode's agent system:

### When to use subagents
- File creation/management operations → use `file-processor` agent
- Data analysis tasks → use `data-analyst` agent  
- Code generation tasks → use `code-generator` agent
- Multi-agent workflows → coordinate between multiple agents

### Example subagent usage
```yaml
agent: file-processor
description: Creates, modifies, or manages files and directories
```

The skill will automatically route requests to the appropriate subagent when the frontmatter `agent` field is specified.

## Testing

### Test checklist
- [ ] Skill validates in OpenCode
- [ ] Correct YAML frontmatter
- [ ] Agent integration works
- [ ] Examples are functional
- [ ] Documentation is complete

### Usage examples
```bash
# Create a new skill
evolve skill create "my-awesome-skill" --agent "file-processor"

# Test the skill
evolve skill test "my-awesome-skill"

# List available agents
evolve agents list
```

## Best practices for Skill authors

1. **One Skill, one purpose**: Don't create mega-Skills
2. **Specific descriptions**: Include trigger words users will say
3. **Clear instructions**: Write for OpenCode, not humans
4. **Concrete examples**: Show real code, not pseudocode
5. **List dependencies**: Mention required packages in description
6. **Test with teammates**: Verify activation and clarity
7. **Version your Skills**: Document changes in content
8. **Use progressive disclosure**: Put advanced details in separate files
9. **Subagent integration**: Use appropriate agent routing for complex workflows

## Troubleshooting

**Skill doesn't activate**:
- Make description more specific with trigger words
- Include file types and operations in description
- Check file location in `.opencode/skills/`
- Verify YAML syntax and frontmatter

**Subagent not found**:
- Check `evolve agents list` for available agents
- Verify agent name spelling in frontmatter
- Ensure target agent is installed/enabled

**Skill has errors**:
- Check YAML syntax (no tabs, proper indentation)
- Verify file paths (use forward slashes)
- Ensure scripts have execute permissions
- List all dependencies
- Validate subagent configuration

## Examples

### Simple skill (no subagents)
```yaml
---
name: todo-manager
description: Manage TODO items across project files with smart filtering and prioritization. Use when user mentions TODOs, task tracking, or project management.
---
```

### Complex skill with subagent integration
```yaml
---
name: data-importer
description: Import and process data from various formats (CSV, JSON, Excel) with validation and transformation. Use when user mentions data import, file conversion, or data processing.
agent: file-processor
requires-plugins: continual-learning
---
```

## Output format

When creating a Skill, I will:

1. Ask clarifying questions about scope and requirements
2. Suggest a Skill name and location
3. Create the SKILL.md file with proper frontmatter
4. Include clear instructions and examples
5. Add supporting files if needed
6. Provide testing instructions
7. Validate against all requirements
8. Configure subagent routing if needed

The result will be a complete, working Skill that follows all best practices and validation rules.