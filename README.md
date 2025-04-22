# Simple Vector
Simple RAII-style wrapper around self-made pointer array, similar to `std::vector`

## Features
- Inbuilt implementation of pointer array
- Seven constructors
	- Default
	- Default with desired size
	- Default with desired size and value
	- Move
	- Copy
	- Empty with desired capacity
	- Using `std::initalizer_list`
- `PushBack()` for adding element at the end
- `PopBack()` for removing last element
- `Insert()` for adding element in desired position
- `Erase()` for removing element in desired position
- `Reserve()` for increasing capacity
- `Clear()` for zeroing the size
- `GetCapacity()` for checking current capacity
- `GetSize()` and `IsEmpty()` for checking current size and emptiness
- `At()` returning const/non-const reference with bounds check
- `operator[]()` returning const/non-const reference
- `swap()` for swaping contents of two SimpleVectors
- `begin()` and `end()` const/non-const iterators
- Private function `PreInsert()` which returns proper iterator for `Insert()`

## Usage
- Min. C++ version: C++17
- Clone repository and use similarly to `std::vector`. `main.cpp` contains tests and examples
