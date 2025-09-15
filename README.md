# Nova Script Documentation

---

## 📘 Table of Contents

1. [Introduction](#1-introduction)  
2. [Language Features](#2-language-features)  
   - [2.1 Print Statements (`say`)](#21-print-statements-say)  
   - [2.2 Variable Declarations (`let`)](#22-variable-declarations-let)  
   - [2.3 Functions (`define`)](#23-functions-define)  
   - [2.4 Conditionals (`when`, `otherwise`)](#24-conditionals-when-otherwise)  
   - [2.5 Loops (`while`, `for`)](#25-loops-while-for)  
   - [2.6 Comments](#26-comments)  
   - [2.7 Error Handling (`try`, `catch`)](#27-error-handling-try-catch)  
3. [Operators](#3-operators)  
4. [Examples](#4-examples)  
5. [File Execution](#5-file-execution)

---

## 1. Introduction

Nova Script is a simple, readable, English-like programming language designed for beginners and rapid prototyping. It emphasizes clarity and natural language syntax.  
Checkout this LinkedIN post for more info:  
https://www.linkedin.com/feed/update/urn:li:activity:7339534452844281857/

---

## 2. Language Features

### 2.1 Print Statements (`say`)

```ns
say "Hello, world!"
say x
```

* **Syntax**: `say <expression>`
* Prints the value of the expression to the console.

### 2.2 Variable Declarations (`let`)

```ns
let count be 42
let name be "Alice"
let value be count
```

* **Syntax**: `let <identifier> [be= <expression>]`
* Declares a variable, optionally with a value and type.

### 2.3 Functions (`define`)

```ns
define function sayName(name):
  say "Hello, "
  say name

call sayName("Novascript")
```

* **Syntax**: `define function <function-name> [with <param> as <type>, ...]:`
* Defines a function.

### 2.4 Conditionals (`when`,`otherwise`)

```ns
let age be 18
when age<18 then
    say "You can't vote"
otherwise
    say "You can vote"
end
```

* **Syntax**:

  * `when <condition> then`
  * `otherwise`

### 2.5 Loops (`while`, `for`)

```ns
#while loop
repeat while a<10
    say a
    set a to a+1
end

#for
repeat for j from 1 to 5
    say j
end
```

* **Syntax**:

  * `repeat while <condition>`
  * `repeat for <variable> from <start> to <end>:`

### 2.6 Comments

```ns
# This is a single-line comment
/*
THis is a multiline comment.
*/
```

* Lines starting with `#` are ignored.

### 2.7 Error Handling (`try`, `catch`)

```ns
try:
  let x be 10 / 0
catch error:
  say "Error: " + error
```

* **Syntax**:

  * `try:`
  * `catch <error-var>:`

---


## 3. Operators

* `+`, `-`, `*`, `/`, `==`, `!=`, `>`, `<`, `>=`, `<=`

---

## 4. Examples

### Factorial

```ns
define function factorial(n)
  let result be 1
  let i be 1

  repeat while i<=n
    set result to result*i
    set i to i+1
  end

  return result
end

let answer be call factorial(5)
say answer
```

### Fibonacci

```ns
define function fibonacci(n)
  let a be 0
  let b be 1
  let c be 0

  repeat while c<n
    say a
    let temp be b
    set b to a+b
    set a to temp
    set c to c+1
  end
end

call fibonacci(10)
```
### Exception Handling

```ns
try
  let number be 0
  let result be 100/number
  say result
catch error
  say "an error occured:"
  say error
end


```

---

## 5. File Execution

Save code in a `.ns` file and run:

```terminal
g++ -I../include -o main main.cpp lexer.cpp Parser.cpp SymbolTable.cpp SemanticAnalyzer.cpp -std=c++17 -mconsole
```
After successful compilation - run:
```
./main
```
Bam! You just experienced Novascript!
---

