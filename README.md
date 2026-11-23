# Library Lending System

A C++17 project that demonstrates OOP through a single borrowing and returns workflow. A library registers members, looks up items, tracks active loans, rejects duplicate borrowing, and makes a returned item available again.

## Run

```sh
g++ -std=c++17 -Wall -Wextra -Wpedantic main.cpp -o library
./library
./library --test
```

The executable runs a scripted demo. A member borrows a book for 14 days and a magazine for 7 days, tries to borrow the same book twice, and returns it. The catalogue displays which items are available. State is in memory.

| Concept | Where to see it |
|---|---|
| Class and object | `Library` defines behavior, and `library` in `main` is an instance. |
| Encapsulation | The catalogue, member records, and loans are private. `borrow()` and `returnItem()` enforce the lending rules. |
| Abstraction | `LibraryItem` defines the common interface through pure virtual functions. |
| Inheritance | `Book` and `Magazine` derive from `LibraryItem`. |
| Runtime polymorphism and overriding | `borrow()` calls `loanDays()` through a `LibraryItem` reference. The actual item determines the loan period. |
| Compile-time polymorphism and overloading | `find(int)` searches by ID. `find(const std::string&)` searches by title. |
| Composition | `Library` owns its catalogue items using `std::unique_ptr` and stores its loan records by value. |
| Association | A loan links a member ID to the item ID used as the key in the loan map. |
| Constructors and virtual destructor | Constructors establish valid initial state. The virtual destructor allows safe cleanup through a base pointer. |

To practice extending the design, add a `DVD` with a three-day loan and register it in `seedCatalog()`. The existing borrowing logic can call its new `loanDays()` implementation through the same interface.
