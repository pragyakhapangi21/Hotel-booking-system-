#include <iostream>
#include <fstream>
#include <string>
using namespace std;

struct Room {
    int roomNo;
    string type;
    double price;
    bool isBooked;
};

struct Booking {
    string customerName;
    string phone;
    int roomNo;
    string checkIn;
    string checkOut;
    string paymentMethod;
};

Room rooms[10];
Booking bookings[10];
int bookingCount = 0;

bool adminLogin() {
    string savedUser, savedPass;
    ifstream file("admin.txt");

    // ---------------- FIRST TIME LOGIN (SET USERNAME & PASSWORD) ----------------
    if (!file) {
        cout << "\n===== SET ADMIN ACCOUNT (FIRST TIME) =====\n";
        cout << "Create Username: ";
        cin >> savedUser;
        cout << "Create Password: ";
        cin >> savedPass;

        ofstream newFile("admin.txt");
        newFile << savedUser << endl << savedPass;
        newFile.close();

        cout << "Admin account created successfully!\n";
        return true;
    }

    // ---------------- NORMAL LOGIN (AFTER FIRST TIME) ----------------
    file >> savedUser >> savedPass;
    file.close();

    cout << "\n===== ADMIN LOGIN =====\n";
    string user, pass;
    cout << "Username: ";
    cin >> user;
    cout << "Password: ";
    cin >> pass;

    if (user == savedUser && pass == savedPass) {
        cout << "Login successful!\n";
        return true;
    } else {
        cout << "Invalid username or password.\n";
        return false;
    }
}

// ---------------------- CREATE ROOMS ----------------------
void createRooms() {
    string types[3] = {"Standard", "Deluxe", "Suite"};
    double prices[3] = {2000, 3500, 5000};

    for (int i = 0; i < 10; i++) {
        rooms[i].roomNo = 101 + i;
        rooms[i].type = types[i % 3];
        rooms[i].price = prices[i % 3];
        rooms[i].isBooked = false;
    }
}

// ---------------------- SAVE TO FILE ----------------------
void saveToFile() {
    ofstream file("bookings.txt");
    file << bookingCount << endl;

    for (int i = 0; i < bookingCount; i++) {
        file << bookings[i].customerName << endl;
        file << bookings[i].phone << endl;
        file << bookings[i].roomNo << endl;
        file << bookings[i].checkIn << endl;
        file << bookings[i].checkOut << endl;
        file << bookings[i].paymentMethod << endl;
    }

    // Save room status
    for (int i = 0; i < 10; i++) {
        file << rooms[i].isBooked << endl;
    }

    file.close();
}

// ---------------------- LOAD FROM FILE ----------------------
void loadFromFile() {
    ifstream file("bookings.txt");
    if (!file) return;

    file >> bookingCount;
    file.ignore();

    for (int i = 0; i < bookingCount; i++) {
        getline(file, bookings[i].customerName);
        getline(file, bookings[i].phone);
        file >> bookings[i].roomNo;
        file.ignore();
        getline(file, bookings[i].checkIn);
        getline(file, bookings[i].checkOut);
        getline(file, bookings[i].paymentMethod);
    }

    // Load room booking statuses
    for (int i = 0; i < 10; i++) {
        file >> rooms[i].isBooked;
    }

    file.close();
}

// ---------------------- SORT ROOMS ----------------------
void sortRooms() {
    for (int i = 0; i < 9; i++) {
        for (int j = i + 1; j < 10; j++) {
            if (rooms[i].price > rooms[j].price) {
                Room temp = rooms[i];
                rooms[i] = rooms[j];
                rooms[j] = temp;
            }
        }
    }
}

// ---------------------- VIEW ROOMS ----------------------
void viewRooms() {
    cout << "\n===== ROOM LIST (Sorted by Price) =====\n";
    sortRooms();
    for (int i = 0; i < 10; i++) {
        cout << "Room: " << rooms[i].roomNo
             << " | Type: " << rooms[i].type
             << " | Price: Rs. " << rooms[i].price
             << " | Status: " << (rooms[i].isBooked ? "Booked" : "Available")
             << endl;
    }
}

// ---------------------- ADD BOOKING ----------------------
void addBooking() {
    if (bookingCount >= 10) {
        cout << "Booking full!\n";
        return;
    }

    Booking b;
    cout << "\nEnter Customer Name: ";
    cin.ignore();
    getline(cin, b.customerName);

    cout << "Phone: ";
    getline(cin, b.phone);

    viewRooms();
    cout << "Enter Room Number to Book: ";
    cin >> b.roomNo;

    bool found = false;
    for (int i = 0; i < 10; i++) {
        if (rooms[i].roomNo == b.roomNo && !rooms[i].isBooked) {
            rooms[i].isBooked = true;
            found = true;
            break;
        }
    }

    if (!found) {
        cout << "Room not available!\n";
        return;
    }

    cout << "Check-in Date (DD/MM/YYYY): ";
    cin >> b.checkIn;

    cout << "Check-out Date (DD/MM/YYYY): ";
    cin >> b.checkOut;

    cout << "Payment Method (Cash/Online/Card): ";
    cin >> b.paymentMethod;

    bookings[bookingCount++] = b;

    saveToFile();  // SAVE
    cout << "Booking successful!\n";
}

// ---------------------- SEARCH CUSTOMER ----------------------
void searchCustomer() {
    string name;
    cout << "\nEnter customer name to search: ";
    cin.ignore();
    getline(cin, name);

    for (int i = 0; i < bookingCount; i++) {
        if (bookings[i].customerName == name) {
            cout << "\nCustomer Found!\n";
            cout << "Room: " << bookings[i].roomNo << endl;
            cout << "Phone: " << bookings[i].phone << endl;
            cout << "Check-In: " << bookings[i].checkIn << endl;
            cout << "Check-Out: " << bookings[i].checkOut << endl;
            return;
        }
    }
    cout << "Customer not found.\n";
}

// ---------------------- DELETE BOOKING ----------------------
void deleteBooking() {
    int room;
    cout << "\nEnter room number to delete booking: ";
    cin >> room;

    for (int i = 0; i < bookingCount; i++) {
        if (bookings[i].roomNo == room) {

            for (int j = 0; j < 10; j++) {
                if (rooms[j].roomNo == room)
                    rooms[j].isBooked = false;
            }

            for (int k = i; k < bookingCount - 1; k++) {
                bookings[k] = bookings[k + 1];
            }

            bookingCount--;
            saveToFile(); // SAVE
            cout << "Booking deleted.\n";
            return;
        }
    }
    cout << "Booking not found.\n";
}

// ---------------------- MAIN MENU ----------------------
void menu() {
    int ch;
    do {
        cout << "\n===== HOTEL BOOKING SYSTEM =====\n";
        cout << "1. View Rooms\n";
        cout << "2. Add Booking\n";
        cout << "3. Search Customer\n";
        cout << "4. Delete Booking\n";
        cout << "5. Exit\n";
        cout << "Choose option: ";
        cin >> ch;

        switch (ch) {
            case 1: viewRooms(); break;
            case 2: addBooking(); break;
            case 3: searchCustomer(); break;
            case 4: deleteBooking(); break;
            case 5: cout << "Goodbye!\n"; break;
            default: cout << "Invalid option.\n";
        }
    } while (ch != 5);
}

int main() {
    if (!adminLogin()) return 0;

    createRooms();
    loadFromFile();  // LOAD SAVED DATA

    menu();
    return 0;
}

