#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <queue>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <exception>
using namespace std;

class Passenger {
public:
    int passengerID;
    int age;
    string gender;
    bool family;
    static vector<Passenger> passengers;
    Passenger() {}
    Passenger(int pid, int a, string g, bool f) : passengerID(pid), age(a), gender(g), family(f) {}

    friend ostream& operator<<(ostream& o, Passenger& p) {
        o << "PassengerID: " << p.passengerID << "  Age: " << p.age << " Gender: " << p.gender << "  Family: " << p.family << endl;
        return o;
    }

    virtual void getDetails() const {
        cout << "Passenger ID: " << passengerID << endl;
        cout << "Age: " << age << endl;
        cout << "Gender: " << gender << endl;
        cout << "Family: " << (family ? "Yes" : "No") << endl;
    }

    void savePassenger(ofstream& file) const {
        file.write(reinterpret_cast<const char*>(&passengerID), sizeof(passengerID));
        file.write(reinterpret_cast<const char*>(&age), sizeof(age));

        size_t size = gender.size();
        file.write(reinterpret_cast<const char*>(&size), sizeof(size));
        file.write(gender.c_str(), size);

        file.write(reinterpret_cast<const char*>(&family), sizeof(family));
    }

    void readPassenger(ifstream& file) {
        file.read(reinterpret_cast<char*>(&passengerID), sizeof(passengerID));
        file.read(reinterpret_cast<char*>(&age), sizeof(age));

        size_t size;
        file.read(reinterpret_cast<char*>(&size), sizeof(size));
        gender.resize(size);
        file.read(&gender[0], size);

        file.read(reinterpret_cast<char*>(&family), sizeof(family));
    }

};

vector<Passenger> Passenger::passengers;

class PassengerAdult : public Passenger {
public:
    PassengerAdult(int pid, int a, string g, bool f) : Passenger(pid, a, g, f) {}
};

class PassengerChild : public Passenger {
public:
    PassengerChild(int pid, int a, string g, bool f) : Passenger(pid, a, g, f) {}
};

class Seat {
public:
    string seatno;
    Passenger assignedpassenger;
    Seat() {}
    Seat(string s, Passenger p) : seatno(s), assignedpassenger(p) {}

    friend ostream& operator<<(ostream& o, Seat& s) {
        o << "SeatNo: " << s.seatno << "  Passenger: " << s.assignedpassenger.passengerID << endl;
        return o;
    }

    void displaySeat() const {
        cout << "Seat [Row: " << seatno[0] << ", Col: " << seatno[1] << "] - Passenger: "
            << assignedpassenger.passengerID << endl;
    }

    void saveSeat(ofstream& file) const {
        size_t size = seatno.size();
        file.write(reinterpret_cast<const char*>(&size), sizeof(size));
        file.write(seatno.c_str(), size);
        assignedpassenger.savePassenger(file);
    }

    void readSeat(ifstream& file) {
        size_t size;
        file.read(reinterpret_cast<char*>(&size), sizeof(size));
        seatno.resize(size);
        file.read(&seatno[0], size);
        assignedpassenger.readPassenger(file);
    }

};

class SeatingPlan {
private:
    vector<vector<string>> plan;
    queue<vector<Seat>> previousPlans;

public:
    int id; // auto-generated
    int numRows;
    int numCols;
    vector<Seat> seats; // contains all the seats and passengers sitting there
    vector<string> seatsStr = { "A1", "B1", "C1", "D1", "A2", "B2", "C2", "D2", "A3", "B3", "C3", "D3", "A4", "B4", "C4", "D4", "A5", "B5", "C5", "D5", "A6", "B6", "C6", "D6", "A7", "B7", "C7", "D7", "A8", "B8", "C8", "D8" };
    vector<string> femaleSeats = { "A1", "B1", "C1", "D1", "A2", "B2", "C2", "D2", "A3", "B3", "C3", "D3", "A4", "B4", "C4", "D4" };
    vector<string> maleSeats = { "A5", "B5", "C5", "D5", "A6", "B6", "C6", "D6", "A7", "B7", "C7", "D7", "A8", "B8", "C8", "D8" };
    vector<string> childSeats = { "A1", "B1", "C1", "A2", "B2", "C2", "D2", "A3", "B3", "C3", "D3", "A4", "B4", "C4", "A5", "B5", "C5", "A6", "B6", "C6", "D6", "A7", "B7", "C7", "D7", "A8", "B8", "C8" };
    vector<string> doorSeats = { "D1", "D4", "D5", "D8" };

    static int nextID;

    SeatingPlan() : plan(8, vector<string>(2, "Empty")), numRows(8), numCols(2) {} //vector of all the seats in the plane

    SeatingPlan(int rows, int cols) : numRows(rows), numCols(cols) {}

    bool operator<(const SeatingPlan& other) const {
        return this->id < other.id;
    }

    void displayRules() const {
        cout << "Seating Allocation Rules:" << endl;
        cout << "1. Families and ladies must first be seated in the Family & Ladies section." << endl;
        cout << "2. Children must be safely seated away from the door." << endl;
    }

    void displayPlan() const {
        if (seats.empty()) {
            cout << "No current seating plan." << endl;
            return;
        }
        cout << "Displaying seating plan..." << endl;
        for (const auto& seat : seats) {
            cout << "Seat: " << seat.seatno << ", Passenger: " << seat.assignedpassenger.passengerID << endl;
        }
    }


    void generateNewPlan() {
        resetPlan(); // reset plan and seats
        bookFlight();
        cout << endl << "This is your current plan." << endl;
        displayPlan();
        cout << endl << "If you want to save this plan, press S" << endl;
        cout << "OR If you want to reset your plan, press R." << endl;
    }

    void resetPlan() {
        for (auto& row : plan) {
            fill(row.begin(), row.end(), "Empty");
        }
        seats.clear();
    }

    void removeSeatFromAllVectors(const string& seat) {
        auto removeSeat = [&seat](vector<string>& seats) {
            auto it = find(seats.begin(), seats.end(), seat);
            if (it != seats.end()) {
                seats.erase(it);
            }
            };

        removeSeat(seatsStr);
        removeSeat(femaleSeats);
        removeSeat(maleSeats);
        removeSeat(childSeats);
        removeSeat(doorSeats);
    }


    string allocateSeat(vector<string>& seats) {
        if (seats.empty()) {
            throw runtime_error("Sorry, there are no available seats.");
        }

        int index = rand() % seats.size();
        string seat = seats[index];
        removeSeatFromAllVectors(seat); // Remove seat from all vectors
        return seat;
    }


    void allocateFamilySeats(int family_size) {
        if (seatsStr.size() < family_size) {
            throw runtime_error("Not enough seats available for the entire family.");
        }

        cout << "Seats assigned for your family: ";
        int allocated = 0;
        vector<string> seatsToRemove;

        for (auto iter = seatsStr.begin(); iter != seatsStr.end() && allocated < family_size; ) {
            if (find(doorSeats.begin(), doorSeats.end(), *iter) == doorSeats.end()) {
                cout << *iter << " ";
                seatsToRemove.push_back(*iter);

                // Create and store the Passenger object in the passengers vector
                Passenger passenger(Passenger::passengers.size() + 1, 0, "", true); // Assuming family members have the same age and gender is not specified
                Passenger::passengers.push_back(passenger);

                // Add assigned seat to the seats vector
                seats.push_back(Seat(*iter, passenger));

                iter = seatsStr.erase(iter);
                allocated++;
            }
            else {
                iter++;
            }
        }
        cout << endl;

        for (const auto& seat : seatsToRemove) {
            removeSeatFromAllVectors(seat);
        }

        if (allocated < family_size) {
            throw runtime_error("Not enough non-door seats available for the entire family.");
        }
    }

    void bookFlight() {
        srand(static_cast<unsigned int>(time(0)));
        int family;
        string gender;
        int age, family_size;

        while (true) {
            cout << "Are you booking for a family? (1 for yes, 0 for no, -1 if you're done): ";
            cin >> family;

            if (cin.fail()) {
                cerr << "Invalid input. Please enter a number (1, 0, or -1)." << endl;
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }

            if (family == -1) {
                return; // Exit the function properly
            }

            if (family == 1) {
                cout << "Enter family size: ";
                cin >> family_size;
                if (cin.fail()) {
                    cerr << "Invalid input. Please enter a valid number for family size." << endl;
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    continue;
                }
                try {
                    allocateFamilySeats(family_size);
                    cout << "Family seats allocated successfully." << endl;
                }
                catch (const runtime_error& e) {
                    cerr << e.what() << endl;
                    continue; // Prompt the user again
                }
            }
            else if (family == 0) {
                try {
                    cout << "Enter gender (Male/Female): ";
                    cin >> gender;
                    if (cin.fail()) {
                        cerr << "Invalid input. Please enter a valid gender." << endl;
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        continue;
                    }
                    if (gender != "Female" && gender != "female" && gender != "F" && gender != "f" && gender != "Male" && gender != "male" && gender != "M" && gender != "m") {
                        throw runtime_error("Invalid gender input.");
                    }

                    cout << "Enter age: ";
                    cin >> age;
                    if (cin.fail()) {
                        cerr << "Invalid input. Please enter a valid number for age." << endl;
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        continue;
                    }
                    if (age < 18) {
                        throw runtime_error("Children cannot travel alone.");
                    }

                    string seat;

                    if (gender == "Female" || gender == "female" || gender == "F" || gender == "f") {
                        if (!femaleSeats.empty()) {
                            seat = allocateSeat(femaleSeats);
                        }
                        else {
                            seat = allocateSeat(maleSeats);
                        }
                    }
                    else if (gender == "Male" || gender == "male" || gender == "M" || gender == "m") {
                        if (!maleSeats.empty()) {
                            seat = allocateSeat(maleSeats);
                        }
                        else if (!femaleSeats.empty()) {
                            throw runtime_error("Men cannot sit in the ladies' and families' section.");
                        }
                        else {
                            seat = allocateSeat(seatsStr);
                        }
                    }

                    cout << "Seat assigned: " << seat << endl;
                    Passenger passenger(Passenger::passengers.size() + 1, age, gender, false);
                    Passenger::passengers.push_back(passenger);
                    seats.push_back(Seat(seat, passenger));
                }
                catch (const runtime_error& e) {
                    cerr << e.what() << endl;
                    continue; // Prompt the user again
                }
            }
            else {
                cerr << "Invalid input. Please enter 1, 0, or -1." << endl;
            }
        }
    }

    void retrievePreviousPlans() {
        // Check if there are any previous plans
        if (previousPlans.empty()) {
            cout << "No previous seating plans available." << endl;
            return;
        }

        // Display each previous plan
        int planNumber = 1;
        queue<vector<Seat>> tempQueue = previousPlans; // Create a copy of the queue to iterate through
        while (!tempQueue.empty()) {
            cout << "Seating Plan " << planNumber << ":" << endl;
            const vector<Seat>& planSeats = tempQueue.front();

            // Display seats for this plan
            for (const auto& seat : planSeats) {
                cout << "Seat: " << seat.seatno << ", Passenger ID: " << seat.assignedpassenger.passengerID << endl;
            }

            cout << endl;
            tempQueue.pop(); // Move to the next plan in the queue
            planNumber++;
        }
    }


    void savePlan() {
        // Convert current seats to vector and push into previousPlans
        vector<Seat> currentSeats = seats;
        if (previousPlans.size() >= 5) {
            previousPlans.pop(); // Remove the oldest plan
        }
        previousPlans.push(currentSeats);

        // Rewrite the file with plans from previousPlans queue
        ofstream file("seating_plan.bin", ios::binary);
        if (file.is_open()) {
            queue<vector<Seat>> tempQueue = previousPlans; // Create a copy of the queue

            while (!tempQueue.empty()) {
                const vector<Seat>& currentSeats = tempQueue.front();

                // Write the number of seats
                size_t seatCount = currentSeats.size();
                file.write(reinterpret_cast<const char*>(&seatCount), sizeof(seatCount));

                // Write each seat
                for (const auto& seat : currentSeats) {
                    seat.saveSeat(file);
                }

                tempQueue.pop(); // Move to the next plan in the queue
            }

            file.close();
            cout << "Seating plans saved successfully." << endl;
        }
        else {
            cerr << "Error: Unable to open file for saving." << endl;
        }
    }


    void readPlans() {
        ifstream file("seating_plan.bin", ios::binary);
        if (file.is_open()) {
            while (!file.eof()) {
                size_t seatCount;
                file.read(reinterpret_cast<char*>(&seatCount), sizeof(seatCount));
                if (file.eof()) break;

                vector<Seat> loadedSeats;
                for (size_t i = 0; i < seatCount; ++i) {
                    Seat seat;
                    seat.readSeat(file);
                    loadedSeats.push_back(seat);
                }

                previousPlans.push(loadedSeats);
            }

            file.close();
            cout << "Seating plans read successfully." << endl;
        }
        else {
            cerr << "Error: Unable to open file for reading." << endl;
        }
    }


}; // class SeatingPlan

int SeatingPlan::nextID = 1; // Initialize static member variable

class Menu {
public:
    static void showMenu() {
        cout << "\nMenu:" << endl;
        cout << "G. Generate New Seating Plan" << endl;
        cout << "D. Display Current Seating Plan" << endl;
        cout << "R. Reset Current Seating Plan" << endl;
        cout << "S. Save Current Seating Plan" << endl;
        cout << "P. Retrieve Previous Seating Plans" << endl;
        cout << "Q. Quit" << endl;
        cout << "Enter your choice: ";
    }

    static void handleChoice(char choice, SeatingPlan& sp) {
        switch (toupper(choice)) {
        case 'G':
            sp.generateNewPlan();
            break;
        case 'D':
            sp.displayPlan();
            break;
        case 'R':
            sp.resetPlan();
            cout << "Seating plan has been reset to default." << endl;
            break;
        case 'S':
            sp.savePlan();
            break;
        case 'P':
            sp.retrievePreviousPlans();
            break;
        case 'Q':
            cout << "Exiting program." << endl;
            exit(0);
        default:
            cout << "Invalid choice. Please try again." << endl;
        }
    }
};

int main() {
    SeatingPlan sp;
    sp.readPlans();
    char choice;
    while (true) {
        cout << endl;
        sp.displayRules();
        cout << endl;
        Menu::showMenu();

        cin >> choice;
        Menu::handleChoice(choice, sp);
    }
    return 0;
}