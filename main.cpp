#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

class LibraryItem {
public:
    LibraryItem(int id, std::string title) : id_(id), title_(std::move(title)) {
        if (id <= 0 || title_.empty()) {
            throw std::invalid_argument("An item needs a positive ID and a title");
        }
    }
    virtual ~LibraryItem() = default;
    LibraryItem(const LibraryItem&) = delete;
    LibraryItem& operator=(const LibraryItem&) = delete;

    int id() const { return id_; }
    const std::string& title() const { return title_; }
    virtual std::string kind() const = 0;
    virtual int loanDays() const = 0;

private:
    int id_;
    std::string title_;
};

class Book final : public LibraryItem {
public:
    using LibraryItem::LibraryItem;
    std::string kind() const override { return "Book"; }
    int loanDays() const override { return 14; }
};

class Magazine final : public LibraryItem {
public:
    using LibraryItem::LibraryItem;
    std::string kind() const override { return "Magazine"; }
    int loanDays() const override { return 7; }
};

class Member {
public:
    explicit Member(std::string name) : name_(std::move(name)) {
        if (name_.empty()) {
            throw std::invalid_argument("A member needs a name");
        }
    }
    const std::string& name() const { return name_; }

private:
    std::string name_;
};

class Loan {
public:
    Loan(int memberId, int days) : memberId_(memberId), days_(days) {}
    int memberId() const { return memberId_; }
    int days() const { return days_; }

private:
    int memberId_;
    int days_;
};

class Library {
public:
    void add(std::unique_ptr<LibraryItem> item) {
        if (!item) {
            throw std::invalid_argument("An item is required");
        }
        const int id = item->id();
        if (!catalog_.try_emplace(id, std::move(item)).second) {
            throw std::invalid_argument("Duplicate item ID");
        }
    }

    int registerMember(std::string name) {
        const int id = nextMemberId_;
        members_.try_emplace(id, std::move(name));
        ++nextMemberId_;
        return id;
    }

    const LibraryItem& find(int itemId) const {
        return *catalog_.at(itemId);
    }

    const LibraryItem& find(const std::string& title) const {
        const LibraryItem* match = nullptr;
        for (const auto& entry : catalog_) {
            if (entry.second->title() == title) {
                if (match) {
                    throw std::invalid_argument("Several items share that title; use an ID");
                }
                match = entry.second.get();
            }
        }
        if (!match) {
            throw std::out_of_range("Title not found");
        }
        return *match;
    }

    Loan borrow(int memberId, int itemId) {
        members_.at(memberId);
        const auto& item = find(itemId);
        const auto [entry, inserted] = loans_.try_emplace(itemId, memberId, item.loanDays());
        if (!inserted) {
            throw std::logic_error("Item is already on loan");
        }
        return entry->second;
    }

    void returnItem(int memberId, int itemId) {
        const auto& loan = loans_.at(itemId);
        if (loan.memberId() != memberId) {
            throw std::logic_error("Only the borrowing member can return this item");
        }
        loans_.erase(itemId);
    }

    void printCatalog(std::ostream& output) const {
        for (const auto& [id, item] : catalog_) {
            output << id << " | " << item->kind() << " | " << item->title() << " | "
                   << (loans_.count(id) ? "On loan" : "Available") << '\n';
        }
    }

private:
    int nextMemberId_ = 1;
    std::map<int, std::unique_ptr<LibraryItem>> catalog_;
    std::map<int, Member> members_;
    std::map<int, Loan> loans_;
};

void seedCatalog(Library& library) {
    library.add(std::make_unique<Book>(101, "Clean Code"));
    library.add(std::make_unique<Magazine>(201, "Computing Today"));
}

void runChecks() {
    Library library;
    seedCatalog(library);
    const int reader = library.registerMember("Namrutha");
    const int secondReader = library.registerMember("Asha");

    assert(library.find("Clean Code").id() == 101);
    assert(library.find(201).kind() == "Magazine");
    const auto bookLoan = library.borrow(reader, 101);
    const auto magazineLoan = library.borrow(reader, 201);
    assert(bookLoan.days() == 14);
    assert(magazineLoan.days() == 7);

    try {
        library.borrow(secondReader, 101);
        throw std::runtime_error("Duplicate loan was accepted");
    } catch (const std::logic_error&) {
    }

    try {
        library.returnItem(secondReader, 101);
        throw std::runtime_error("Return by the wrong member was accepted");
    } catch (const std::logic_error&) {
    }

    library.returnItem(reader, 101);
    const auto newLoan = library.borrow(secondReader, 101);
    assert(newLoan.memberId() == secondReader);
    std::cout << "All lending checks passed.\n";
}

int main(int argc, char* argv[]) {
    if (argc == 2 && std::string(argv[1]) == "--test") {
        runChecks();
        return 0;
    }

    Library library;
    seedCatalog(library);
    const int memberId = library.registerMember("Namrutha");

    std::cout << "Library catalogue\n";
    library.printCatalog(std::cout);

    const auto& book = library.find("Clean Code");
    const auto bookLoan = library.borrow(memberId, book.id());
    const auto magazineLoan = library.borrow(memberId, 201);
    std::cout << "\nBorrowed " << book.title() << " for " << bookLoan.days() << " days.\n"
              << "Borrowed " << library.find(201).title() << " for "
              << magazineLoan.days() << " days.\n";

    try {
        library.borrow(memberId, 101);
    } catch (const std::logic_error& error) {
        std::cout << error.what() << '\n';
    }

    library.returnItem(memberId, 101);
    std::cout << "\nCatalogue after returning item 101\n";
    library.printCatalog(std::cout);
}
