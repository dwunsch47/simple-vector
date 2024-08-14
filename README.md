Simple Vector
-------------
Simple vector, similar to std::vector, made for educational purpose

Features
--------
- Inbuilt implementation of pointer array for internal use
- Seven constructors
	- Default
	- Default with desired size
	- Default with desired size and value
	- Move
	- Copy
	- Empty with desired capacity
	- Using std::initalizer_list
- PushBack() for adding element at the end
- PopBack() for removing last element
- Insert() for adding element in desired position
- Erase() for removing element in desired position
- Reserve() for increasing capacity
- At() returning const/non-const reference
- Clear() for zeroing the size
- GetCapacity() for checking current capacity
- GetSize() and IsEmpty() for checking current size and emptiness
- operator[]() returning const/non-const reference
- swap() for swaping contents of two SimpleVectors
- begin() and end() const/non-const iterators
- Private function PreInsert() with main Insert() logic

Usage
------
Min. C++ version: C++17

Clone repository and use similarly to std::vector
