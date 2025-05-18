# MyCustomLang - The Human-Friendly Programming Language

## Table of Contents
- [1. Language Basics](#1-language-basics)
  - [1.1 Comments](#11-comments)
  - [1.2 Identifiers](#12-identifiers)
- [2. Variables](#2-variables)
  - [2.1 Declaration](#21-declaration)
  - [2.2 Assignment](#22-assignment)
  - [2.3 Scoping](#23-scoping)
- [3. Control Flow](#3-control-flow)
  - [3.1 Conditionals](#31-conditionals)
  - [3.2 Loops](#32-loops)
- [4. Functions](#4-functions)
  - [4.1 Definition](#41-definition)
  - [4.2 Calling](#42-calling)
  - [4.3 Advanced Features](#43-advanced-features)
- [5. Error Handling](#5-error-handling)
- [6. I/O Operations](#6-io-operations)
- [7. Collections](#7-collections)
- [8. AI/ML Integration](#8-aiml-integration)
- [9. Complete Grammar](#9-complete-grammar)

---

## 1. Language Basics

### 1.1 Comments
```python
# This is a single-line comment

/*
  This is a multi-line
  comment block
*/
```

### 1.2 Identifiers
- Start with letter or underscore
- Can contain letters, numbers, underscores
- Case-sensitive

```python
let myVar1 be 10
set _temp as "value"
```

## 2. Variables

### 2.1 Declaration
```python
let count be 10            # Integer
set message as "Hello"     # String
let is_valid be true     # Boolean
let prices be [1.5, 2.3]   # List
let person be {            # Dictionary
    "name": "Alice",
    "age": 30
}
let bigmyint be 10000L
```

### 2.2 Assignment
```python
set count as count + 1
set person["age"] as 31
set prices[0] as 2.0
```

### 2.3 Scoping
```python
let global be 100

define function test with ()
    let local be 200
    say global + local  # 300
end

define function greet with (name, prefix be "Hello")
    return prefix + ", " + name
end
say local  # Error: undefined
```

## 3. Control Flow

### 3.1 Conditionals
**Basic If-Else**
```python
when x > 0 then
    say "Positive"
otherwise when x < 0 then
    say "Negative"
otherwise
    say "Zero"
end
```

**Pattern Matching**
```python
match response_code
    case 200 then say "OK"
    case 404 then say "Not Found"
    case 500 then say "Server Error"
    case _ then say "Unknown code"
end
```

### 3.2 Loops
**While Loop**
```python
let i be 0
repeat while i < 5
    say i
    increase i by 1
end
```

**For Loop Variations**
```python
# Range-based
repeat for j from 1 to 5
    say j  # 1,2,3,4,5
end

# Full control
repeat with k starting at 10, until k <= 0, step -2
    say k  # 10,8,6,4,2,0
end

# Multi-variable
repeat with (x at 0), (y at 100), until x > y, step (x+5, y-10)
    say f"{x},{y}"
end
```

## 4. Functions

### 4.1 Definition
```python
define function calculate with (a, b, op be "+")
    match op
        case "+" then return a + b
        case "-" then return a - b
        case _ then throw "Invalid operator"
    end
end
```

### 4.2 Calling
```python
let result be call calculate with (5, 3, "-")  # 2
```

### 4.3 Advanced Features
```python
# Higher-order functions
define function apply with (func, x)
    return call func with (x)
end

let square be function with (n) return n * n
say call apply with (square, 4)  # 16

# Variadic arguments
define function sum with (...numbers)
    let total be 0
    repeat for n in numbers
        set total as total + n
    end
    return total
end
```

## 5. Error Handling
```python
try
    let file be open file "data.txt" as read
    let content be read file
    close file
catch error as e
    say "Error occurred: " + e
    try
        close file  # Attempt cleanup
    catch
        say "Failed to close file"
    end
end

define function validate with (age)
    when age < 0 then
        throw "Age cannot be negative"
    end
end
```

## 6. I/O Operations

**Console**
```python
say "Enter your name:"
let name be ask  # Reads input

say f"Hello, {name}! Today is {date()}"
```

**Files**
```python
# Reading
open file "data.json" as read
let json_data be read file as json
close file

# Writing
open file "log.txt" as write
write "New log entry" to file
close file
```

## 7. Collections

**List Operations**
```python
let fruits be ["apple", "banana"]
add "orange" to fruits
remove "banana" from fruits

repeat for fruit in fruits
    say fruit
end
```

**Dictionary Operations**
```python
let user be {
    "name": "Alice",
    "roles": ["admin", "user"]
}

say user["name"]  # Alice
set user["age"] as 30
```

## 8. AI/ML Integration

**Model Definition**
```python
create model "ImageRecognizer"
with architecture:
    type: "CNN"
    layers:
        - input: [28, 28, 1]
        - conv: 32 filters
        - dense: 128 neurons
        - output: 10 neurons
```

**Training Pipeline**
```python
train model using "images/"
with params:
    epochs: 20
    batch_size: 64
    optimizer: "adam"
```

**Prediction**
```python
let results be predict using model with "test_image.png"
show results with confidence > 0.8
```

## 9. Complete Grammar
```
program        → statement*
statement     → expr_stmt | decl_stmt | control_flow | func_def
expr_stmt     → expression
decl_stmt     → LET ID BE expression | SET ID AS expression
control_flow  → if_stmt | while_loop | for_loop | match_stmt
if_stmt       → WHEN expression THEN block (OTHERWISE block)? END
while_loop    → REPEAT WHILE expression block END
for_loop      → REPEAT (FOR ID FROM expression TO expression (STEP expression)?
                         | WITH ID STARTING AT expression UNTIL expression (STEP expression)?)
               block END
func_def      → DEFINE FUNCTION ID WITH "(" params? ")" block END
params        → ID (BE expression)? ("," ID (BE expression)?)*
block         → statement*
```

## Implementation Notes

**Tokenization:**
- Handle multi-word keywords ("repeat while")
- Distinguish between `is` (comparison) and `is` in "is greater than"

**Type System:**
- Dynamic typing by default
- Optional type hints: `let age be 25 as Integer`

**Standard Library:**
- Math functions: `sqrt()`, `sin()`, etc.
- String operations: `split()`, `join()`, etc.
- Time/Date utilities

---

This README includes:
- Detailed syntax for all language constructs
- Multiple examples for each feature
- Edge case considerations
- Complete formal grammar
- Implementation notes

You can save this directly as `README.md` in your project repository.

