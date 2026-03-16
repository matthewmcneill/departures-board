---
description: Add a new item to the project .agents/todo.md list
---

When the user uses the `/todo` command:

- **If followed by text**:
    1. Append a new line to `.agents/todo_list.md`.
    2. The format should be `- [ ] <text provided by the user>`.
    3. If `.agents/todo.md` does not exist, create it with a `# TODO` header first.
    4. Notify the user that the item has been added.

- **If NOT followed by text**:
    1. Open the `.agents/todo_list.md` file using the `view_file` tool to show it to the user.
    2. If the file doesn't exist, notify the user.