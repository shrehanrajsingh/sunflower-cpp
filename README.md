## Sunflower

Sunflower is a small, interpreted programming language implemented in C++ with a focus on:

- **Clean, indentation-based syntax**
- **First-class functions and classes**
- **Rich built-in data structures**
- **Practical standard library for files, sockets, HTTP, threading, and small DBs**

This document summarizes the features that are currently implemented in the language, based on the interpreter (`tokenize`, `ast`, `expr`, `stmt`, `func`, `sfclass`, `sfarray`, `sfdict`, `module`, `environment`, `native`) and the test programs under `tests/*.sf` and `lib/**`.

---

## Installation
To install Sunflower on Linux:

1. **Clone the repository:**
  ```sh
  git clone https://github.com/shrehanrajsingh/sunflower-cpp
  cd sunflower-cpp
  ```

2. **Create a build directory and configure the project:**
  ```sh
  mkdir build
  cd build
  cmake .. -G "Unix Makefiles"
  ```

3. **Build the project:**
  ```sh
  make
  ```

4. **Generate Installer**
  ```sh
  cpack
  ```

This will generate the Sunflower interpreter binary in the `build/bin` directory. You can then run Sunflower programs using this binary.  
`cpack` generates an installer based on the system on which the language is built in.

For additional build options or platform-specific instructions, refer  `CMakeLists.txt`.

---

## Core Syntax

- **Indentation-based blocks**
  - Blocks are introduced by keywords (like `if`, `fun`, `class`, `for`, `while`, `try`, `catch`) and are delimited by indentation, not braces.

  ```text
  if true
      putln ("Inside if")
  else if false
      putln ("Inside else if")
  else
      putln ("Inside else")
  ```

- **Comments**
  - `#` starts a line comment.

  ```text
  # This is a comment
  a = 10  # trailing comment
  ```

- **Variables and assignment**
  - Simple assignment with `=`.
  - Reassignment is allowed, dynamic typing.

  ```text
  a = 10
  b = "Hello, World!"
  c = a
  d = none
  e = true
  f = false
  ```

---

## Data Types and Literals

- **Primitive types**
  - **Integers** and **floats** (tokenized as `Integer` / `Float`).
  - **Booleans**: `true`, `false`.
  - **None**: `none` (distinct `NoneType`).
  - **Strings**: single or double-quoted text.

  ```text
  i = 42
  f = 3.14
  s = "Hello"
  t = true
  n = none
  ```

- **Arrays (lists)**
  - Ordered, indexable, mutable sequences.
  - Support nested arrays, indexing, slicing, comprehension-like construction, and methods like `push`, `pop`, and `clear`.

  ```text
  a = [1, [2, 3], 4, 5]
  x = a[0]
  y = a[1][1]

  # Slices with optional step
  write ("0123456789"[1 to 10 step 2])
  write ([1, 2, 3, 4, 5][0 to 5 step 2])

  # Comprehension-style nested construction
  grid = [ [i, j] for j in 1 to 10 for i in 1 to 10 ]
  ```

- **Dictionaries (maps)**
  - String-keyed maps with nested dictionaries supported.

  ```text
  a = {
      "hello": "hi",
      "a": {
          "a": 1,
          "b": 2
      }
  }

  for key in a
      write (key, a[key])
  ```

- **Class objects**
  - User-defined classes are objects; class values and instances are both first-class.

---

## Expressions and Operators

- **Arithmetic**
  - Standard operators: `+`, `-`, `*`, `/`, `%` (via expression tree in `ExprArith`).
  - Used in loops, math functions, algorithms (e.g. Fibonacci, factorial, TSP).

  ```text
  a = 1
  a = a + 1
  ```

- **Comparisons**
  - `==`, `!=`, `<`, `>`, `<=`, `>=` (see `ConditionalExpr` and `ConditionalType`).

  ```text
  if n == 1
      return n
  if value < self.value
      ...
  ```

- **Logical operators**
  - `and`, `or`, `not` with proper short-circuit semantics (`LogicalAndExpr`, `LogicalOrExpr`, `LogicalNotExpr`).

  ```text
  write (true or false and false)
  write (not true and false)
  write (not (true and false) or false)
  ```

- **Bitwise operators**
  - `&`, `|`, `~`, `<<`, `>>` (`BitAndExpr`, `BitOrExpr`, `BitNegateExpr`, `BitLeftShiftExpr`, `BitRightShiftExpr`).

  ```text
  APPLE  = 1 << 0
  BANANA = 1 << 1

  cart = 0
  cart = cart | APPLE
  cart = cart | BANANA

  if cart & APPLE
      write ("Apple is in cart")

  cart = cart & ~APPLE
  ```

- **Indexing and slicing**
  - `arr[idx]` for arrays and strings (`ArrayAccess`).
  - `start to end step k` as a range expression (`ExprToStep`), used for loops and slices.

  ```text
  write ("a123456789"[0])
  write ("0123456789"[1 to 10 step 2])
  ```

- **Membership / iteration target**
  - `x in y` (via `InExpr`) used in `for`-loops to iterate over arrays, dicts, ranges, etc.

  ```text
  for i in details
      write (i)
  ```

---

## Control Flow

- **If / else-if / else**

  ```text
  if cond
      ...
  else if other_cond
      ...
  else
      ...
  ```

- **While loops**

  ```text
  while n > 1
      t = f2
      f2 = f1 + f2
      f1 = t
      n = n - 1
  ```

- **For loops**
  - General form: `for name in iterable`.
  - Supports arrays, dictionaries (iterating keys), and `1 to N` ranges.

  ```text
  for i in 1 to 30
      nth_fib (i)

  for i in a
      write (i)
  ```

- **Repeat loops**
  - `repeat`-based loops for counting and inline repetition (`RepeatExpr`, `RepeatStatement`).

  ```text
  i = 10
  repeat i
      repeat i
          put ('*')
      i = i - 1
      putln ('')

  # Expression form
  i = 10 repeat 5
  grid = 0 repeat 5 repeat 5
  ```

- **Break and continue**
  - Structured loop control (`BreakStmt`, `ContinueStmt`).

  ```text
  for i in 1 to 10
      for j in 0 to i
          if i == 4 or i == 8
              continue
          put (i)
  ```

---

## Functions

- **Function declarations**
  - `fun` keyword, with positional parameters.

  ```text
  fun test (a)
      write ("Received argument: ", a)
      return a

  test (10)
  ```

- **Return values**
  - `return` for normal returns (`ReturnStatement`).
  - `return ? "message"` or just `? "message"` to raise an error-like value for `try`/`catch` (see **Error Handling**).

  ```text
  fun fact (n)
      if n == 1
          return n
      return fact (n - 1) * n
  ```

- **Short multiple assignment**
  - Supported via array destructuring.

  ```text
  fun nth_fib_short (n)
      [f1, f2] = [0, 1]

      while n > 1
          [f1, f2, n] = [f2, f1 + f2, n - 1]

      return f2
  ```

- **First-class and higher-order usage**
  - Functions can be passed as values, stored in arrays, and used with native modules like threads and sockets.

  ```text
  fun test (a)
      s = 0
      for i in 1 to 10000
          s = s + i
      write ("hi", a)
  ```

---

## Classes and Object-Oriented Features

- **Class declarations**
  - `class Name` with fields and methods.
  - Implicit `self` parameter for instance methods.
  - `_init` is the constructor, invoked on instance creation.

  ```text
  class List
      n = none
      v = none

      fun _init (self, v, n)
          self.n = n
          self.v = v

      fun print (self)
          b = self
          while b
              write (b.v)
              b = b.n

  a = List (10, List (20, List (30, none)))
  a.print ()
  ```

- **More complex classes**
  - Full example with a binary search tree (`TreeNode`), featuring:
    - Methods with recursion
    - Mutating instance fields
    - Multiple traversal algorithms (`inorder_traversal`, `preorder_traversal`, `postorder_traversal`)
    - `search` method using recursion and conditionals.

- **Inheritance and `extends`**
  - Classes can extend other classes using `extends` (see `tests/langfeatures.sf`).

  ```text
  class Rectangle
      w = 0
      h = 0

      fun _init (self, w, h)
          self.w = w
          self.h = h
      
      fun area (self)
          return self.w * self.h

  class Square extends Rectangle
      fun _init (self, s)
          Rectangle._init (self, s, s)
      
      fun perimeter (self)
          return 4 * self.w

  s = Square (5)
  write (s.area ()) # 25
  write (s.perimeter ()) # 20
  ```

---

## Error Handling

- **Raising errors with `?`**
  - `? value` (and `return ? value`) is used to signal an error/exception-like value that can be caught by `try`/`catch`.

  ```text
  fun legal_age (a)
      if a < 18
          return ? "Below legal age"
      return true
  ```

- **Try/Catch as statements**
  - `try` … `catch` blocks for handling raised values (`TryCatchStmt`).

  ```text
  try
      legal_age (17)
  catch E
      if E == "Below legal age"
          write ("<18")
  ```

- **Try/Catch as expressions**
  - Expression-level `try` with an optional catch variable and handler expression (`TryCatchExpr`).

  ```text
  fun age (a)
      if a < 18
          return ? "Below legal age"
      else if a > 999
          return ? "Too old"
      return a

  a = try age (17) catch 18
  write (a)

  a = try age (17) catch e -> e
  write (a)

  a = try age (17) catch e -> {'Below legal age': 18}[e]
  write (a)
  ```

---

## Modules and Imports

- **Import syntax**
  - Import by path or module name with optional alias.

  ```text
  import '_Native_File' as file
  import '../../lib/thread/_init.sf' as thread
  import 'file' as file
  import 'thread' as thread
  import 'socket' as s
  import 'net' as net
  ```

- **Module system**
  - Backed by `Module` objects in C++.
  - `import` supports:
    - Relative `.sf` files
    - Native bindings (e.g. `_Native_File`, `_Native_Socket`, `_Native_Thread`, `_Native_http`)
    - High-level stdlib modules in `lib/**` that wrap native modules in idiomatic Sunflower code.

---

## Standard Library and Native Functions

Many built-ins are provided via the native layer (`native/*.cpp`), and re-exported through simple `.sf` wrappers in `lib/**`.

- **Core I/O**

  - **`input (prompt)`**: read a line from standard input.
  - **`put (value)`**: write to stdout without newline.
  - **`putln (value)`**: write with newline.
  - **`write (...values)`**: formatted/debug printing of multiple values.
  - **`sleep (ms)`**: sleep for a given time in milliseconds.
  - **`len (x)`**: length of strings, arrays, etc.

- **String methods** (via `native/string/*.cpp`)

  - **`s.count (sub)`**: count occurrences of a substring.
  - **`s.find (sub)`**: find index of a substring.
  - **`s.replace (old, new)`**: return a new string with replacements.

- **Integer helpers**

  - **Base conversions** via `int/base` (e.g. formatting/converting integers with arbitrary bases).

- **List methods**

  - **`list.push (value)`**: append.
  - **`list.pop ()`**: pop last element.
  - **`list.clear ()`**: clear the list (used in SmallDB).

- **File API (`lib/file/_init.sf`)**

  - High-level file wrapper around `_Native_File`:

  ```text
  import 'file' as file

  f = file ('./test.txt', 'w')
  f.write ("Hello, World!")
  f.close ()
  ```

  - Methods:
    - `File.read ()`
    - `File.write (s)`
    - `File.seekr (offset, whence)`, `File.seekw (offset, whence)`, `File.seek (offset, whence)`
    - `File.close ()`
  - Errors (e.g. not opened) are reported via `? "..."` so they can be caught by `try`/`catch`.

- **Threading (`lib/thread/_init.sf`)**

  - Thin wrapper over `_Native_Thread` with a `Thread` class and helpers:

  ```text
  import 'thread' as thread

  fun work (id)
      s = 0
      for i in 1 to 10000
          s = s + i
      write ("hi", id)

  t1 = thread (work, [1])
  t2 = thread (work, [2])

  t1.run ()
  t2.run ()

  thread.join_all ()
  ```

  - Features:
    - `Thread (fn, args)` constructor.
    - `run ()`, `join ()`, `_kill ()`.
    - Global `join_all ()` to join all created threads.

- **Sockets (`lib/socket/_init.sf` and native socket)**

  - Low-level `_Native_Socket` and higher-level `socket` module.

  ```text
  import 'socket' as s

  sock = s.socket ()
  sock.bind ('localhost', 8000)
  sock.listen (1000)

  while 1
      try
          con = sock.accept ()
          data = con.read ()
          con.send ("hello")
          con.close ()
      catch E
          write ("Error:", E)
  ```

- **HTTP Server (`lib/http/_init.sf`, `lib/http/server.sf`)**

  ```text
  import 'http' as s
  import 'file' as file

  a = s.Server ('0.0.0.0', 8000)

  fun home ()
      return {
          'status': 200,
          'Content-Type': 'text/html',
          'body': '<h1>Home</h1>'
      }

  fun contact ()
      return {
          'status': 200,
          'Content-Type': 'text/html',
          'body': '<h1>Contact</h1>'
      }

  a.add_get ('/', home)
  a.add_get ('/contact', contact)

  a.serve ()
  ```

---

## Concurrency and Interactive Examples

- **Threaded input / parsing**
  - Example patterns in `tests/langfeatures.sf` show using threads to decouple input and parsing.

  ```text
  import 'thread' as thread

  line = ['']
  got_line = [false]

  fun get_line ()
      while 1
          line[0] = input (">>> ")
          got_line[0] = true

  fun parse ()
      while 1
          while not got_line[0]
              t = 0  # busy-wait

          write ("Input:", line[0])
          got_line[0] = false

  gl_thread = thread (get_line, [])
  p_thread = thread (parse, [])

  gl_thread.run ()
  p_thread.run ()

  thread.join_all ()
  ```

---

## Status

Sunflower is an evolving language with an interpreter and runtime implemented in C++. The features described above are inferred from the current implementation and tests and represent the capabilities that are actively exercised by the codebase. As the language grows, this README should be kept in sync with new syntax, semantics, and standard library modules.