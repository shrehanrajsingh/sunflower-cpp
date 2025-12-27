# TODO: Implement List Methods in Sunflower

## Overview
Implement the following list methods: clear, copy, count, extend, index, insert, remove, reverse, sort.

Each method requires:
- Header file (.hpp) in native/list/
- Implementation file (.cpp) in native/list/
- Update native/native.cpp to include the _add_native_list_* call
- Format code with clang-format -style=GNU -i

## Methods to Implement

### 1. clear()
- Removes all elements from the list
- No arguments
- Returns None

### 2. copy()
- Returns a shallow copy of the list
- No arguments
- Returns new ArrayObject

### 3. count(value)
- Returns number of occurrences of value
- Argument: value (Object*)
- Returns IntegerConstant

### 4. extend(iterable)
- Adds elements from iterable to end of list
- Argument: iterable (Object*, assumed to be ArrayObject)
- Returns None

### 5. index(value)
- Returns index of first occurrence of value
- Argument: value (Object*)
- Returns IntegerConstant or raises error if not found

### 6. insert(index, value)
- Inserts value at specified index
- Arguments: index (int), value (Object*)
- Returns None

### 7. remove(value)
- Removes first occurrence of value
- Argument: value (Object*)
- Returns None or raises error if not found

### 8. reverse()
- Reverses the list in place
- No arguments
- Returns None

### 9. sort()
- Sorts the list in place (default ascending)
- No arguments (for simplicity)
- Returns None

## Progress
- [x] clear.hpp and clear.cpp
- [x] copy.hpp and copy.cpp
- [x] count.hpp and count.cpp
- [x] extend.hpp and extend.cpp
- [x] index.hpp and index.cpp
- [x] insert.hpp and insert.cpp
- [x] remove.hpp and remove.cpp
- [x] reverse.hpp and reverse.cpp
- [x] sort.hpp and sort.cpp
- [x] Update native/native.cpp for all methods
- [x] Format all files with clang-format
