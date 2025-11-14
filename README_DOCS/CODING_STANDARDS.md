# Coding Norms and Style Guide

This project uses an automated script to help ensure our source code is consistent, readable, and easy to maintain. The following rules are checked **automatically for every commit**:

***

## Required Project Header

- Every `.cpp`, `.hpp`, and `.tpp` file **must contain the official school header at the very top**.
- The header must match this format (with dates and filenames as generated):

```
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*                                                    +:+ +:+         +:+     */
/*   By: ouel-bou <ouel-bou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: YYYY/MM/DD HH:MM:SS by ouel-bou          #+#    #+#             */
/*   Updated: YYYY/MM/DD HH:MM:SS by ouel-bou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
```


***

## Function Style

- **Maximum lines per function:** Functions must not exceed **30 lines** (excluding blanks).
- **Naming:**
    - Function names must use `snake_case` or `camelCase`.
    - Function names **must start with a lowercase letter**.
- **Comments:** Each function should be **preceded by a comment** describing its purpose.

***

## Class Style

- **Naming:** All class names **must start with an uppercase letter**.

***

## File Limits

- **Maximum lines per source/header file:** 500 lines.

***

## Indentation \& Formatting

- **Indentation:** Use **4 spaces** for indentation.
- **Only tabs** are allowed (no spaces).

***

## Header Guards

- Every `.hpp` file must begin with the appropriate **header guard** using the uppercase version of the filename, e.g.:

```
#ifndef FILENAME_HPP
#define FILENAME_HPP
... // file content
#endif
```


***

## General Commit Requirements

- Commits will be blocked if:
    - The code **does not compile** with `make re`.
    - **Norm violations** are found by the script on any staged `.cpp`, `.hpp`, or `.tpp` files.

***

## Example Workflow

1. Write or edit code following these norms.
2. Before committing, the pre-commit script will:
    - Try to compile via `make re`.
    - Run the norm script on your staged files.
3. If either check fails, **the commit will be blocked** and you'll see an error telling you what to fix.

***

If you have any questions about these rules, please refer to the script or ask your team lead. Following these norms ensures our code is understandable and maintainable for everyone.\# Team Coding Standards (NORM)

To maintain code quality, all code submissions must pass automated checks enforced by our pre-commit script. Review this guide and always apply these rules before committing.

***

## 1. File Header

- Every `.cpp`, `.hpp`, and `.tpp` file **must start with the school's official header block**.
- The header must exactly include the format with author, creation, and update info (see example below).
- The script checks for:
    - Proper box structure.
    - Author line: `By: ouel-bou <ouel-bou@student.42.fr>`.

***

## 2. Function Rules

- **Function Length:** Maximum 30 lines per function.
- **Naming:** Must be in `snake_case` or `camelCase` and start with a **lowercase** letter.
- **Documentation:** Each function must be **preceded with a one-line comment** that describes its purpose.

***

## 3. Class Rules

- **Naming:** Class names must always start with an **uppercase** letter.

***

## 4. File Limits

- No file (`.cpp`, `.hpp`, `.tpp`) should exceed **500 lines**.

***

## 5. Indentation

- Use **4 spaces** for each indentation level.
- Only tabs are allowed.

***

## 6. Header Guards

- All `.hpp` files must include a matching **header guard** at the top and bottom, based on the capitalized file name.
    - Example for `foo.hpp`:

```
#ifndef FOO_HPP
#define FOO_HPP
 // ...
#endif
```


***

## 7. Pre-commit Script Behavior

- **Compiles first:** Your code must compile with `make re` or the commit will fail.
- **Norm compliance:** Automated script will check for all of the above norms on changed files.
- **Staged only:** Checks apply to files staged for commit (`git add ...`).

***

## Example Header

```
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*                                                    +:+ +:+         +:+     */
/*   By: ouel-bou <ouel-bou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: YYYY/MM/DD HH:MM:SS by ouel-bou          #+#    #+#             */
/*   Updated: YYYY/MM/DD HH:MM:SS by ouel-bou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
```


***

Violations will block your commit. Following these rules ensures a healthy, sustainable codebase for everyone on the team.Here is a descriptive file suitable for sharing the project's norms with your team:

***

# Coding Norms and Project Style Guide

This document summarizes the code standards enforced by our automated pre-commit script.

***

## 1. Required File Header

- Every `.cpp`, `.hpp`, and `.tpp` file must start with the official school header.
- The header block must include your handle and match the expected format (author line, creation and update timestamps).

***

## 2. Function Guidelines

- Each function must not exceed **30 lines**.
- All functions must be named in snake_case or camelCase and start with a lowercase letter.
- Each function requires a brief comment directly above its definition.

***

## 3. Class Naming

- All class names must start with an uppercase letter.

***

## 4. File Limits

- Any code file must be **500 lines or less**.

***

## 5. Indentation

- Indentation is enforced as **4 spaces**.
- Do not use tabs.

***

## 6. Header Guards

- All `.hpp` files must contain a header guard constructed from the filename in uppercase.
    - Example: `#ifndef FILENAME_HPP` / `#define FILENAME_HPP` ... `#endif`

***

## 7. Commit Checks

- The code must compile using `make re` or the commit will be blocked.
- Any norm violation detected by the checker script will prevent the commit.

***

**Always follow these standards to keep the codebase readable and maintainable for everyone.**
<span style="display:none">[^1][^2][^3][^4][^5][^6][^7][^8][^9]</span>

<div align="center">⁂</div>