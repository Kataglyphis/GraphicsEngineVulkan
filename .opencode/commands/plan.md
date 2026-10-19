---
description: Run the planner to analyze the codebase and add tasks to BACKLOG.md
agent: planner
---
Do the planner task the agentic loop gives, single-sourced in the hub:

@third_party/ANTfrastructure/shared/agentic-loop/prompts/planner.md

Completed tasks are deleted from the backlog, so also check `git log --oneline -30`
before adding a task, to avoid re-planning work already done.

$ARGUMENTS
