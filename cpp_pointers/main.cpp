#include <string>
#include <iostream>
#include <sstream>
using namespace std;


static bool debug = false;

class Node {
    private:
        double value;
        int userID;
        int itemID;
        Node* next;

    public:
        Node(int _userID, int _itemID, double _value) {
            userID = _userID;
            itemID = _itemID;
            value = _value;
            next = nullptr;
        };
        Node* getNext() {
            return this->next;
        };
        void setNext(Node* _next) {
            this->next = _next;
        };
        int getUserID() {
            return this->userID;
        };
        int getItemID() {
            return this->itemID;
        };
        void setValue(double _value) {
            this->value = _value;
        };
        double getValue() {
            return this->value;
        };
};

class LinkedList {
    private:
        Node* head;
    public:
        LinkedList() {
            head = nullptr;
        }

        // destructor to free memory
        ~LinkedList() {
            Node* current = head;
            while (current != nullptr) {
                Node* nextNode = current->getNext();
                delete current;
                current = nextNode;
            }
        }

        Node *getHead() {
            return head;
        }
        void setHead(Node* _head) {
            head = _head;
        }
        bool isEmpty() {
            return head == nullptr;
        }


        /*
            If already exists, updates value and returns true
            If not found, returns false
        */
        bool tryUpdate(int _userID, int _itemID, double _value){
            Node* current = head;
            while (current != nullptr) {
                if (current->getUserID() == _userID && current->getItemID() == _itemID) {
                    current->setValue(_value);
                    return true;
                }
                current = current->getNext();
            }
            return false;
        }

        /*
            If already exists, updates the value and returns true
            ( Calls tryUpdate() )
            
            If not found, returns false
        */
        void insert(int _userID, int _itemID, double _value, bool _print = true) {

            // if exists, update the value
            if (!isEmpty() && tryUpdate(_userID, _itemID, _value)) {
                if (_print) cout << "Customer rating (" << _userID << ", " << _itemID << ") is updated" << endl;
                return;
            }

            // add new
            Node *newNode = new Node(_userID, _itemID, _value);

            if (isEmpty()) {
                head = newNode;
                if (_print) cout << "Customer rating (" << _userID << ", " << _itemID << ") is added successful" << endl;
                if (debug) cout << "Debug: Customer rating (User: " << head->getUserID() << ", Item: " << head->getItemID() << ", Value: " << head->getValue() << ") is added as HEAD" << endl;
                return;
            }

            Node* current = head;
            while (current->getNext() != nullptr) {
                current = current->getNext();
            }
            current->setNext(newNode);

            if (debug) cout << "Debug: Customer rating (User: " << newNode->getUserID() << ", Item: " << newNode->getItemID() << ", Value: " << newNode->getValue() << ") is added" << endl;
            if (_print) cout << "Customer rating (" << _userID << ", " << _itemID << ") is added successful" << endl;
        }

        /*
            Removes the rating and returns true
            If not found, returns false
        */
        void remove(int _userID, int _itemID, bool _print = true) {

            if (head == nullptr) {
                if (_print) cout << "Customer rating (" << _userID << "," << _itemID << ") does not exist" << endl; // list null -> display not exists
                return;
            }
            Node* current = head;
            // check if head matches target node
            if (current->getUserID() == _userID && current->getItemID() == _itemID) {
                head = current->getNext();
                delete current;
                if (_print) cout << "Customer rating (" << _userID << ", " << _itemID << ") is removed successful" << endl;
                return;
            }
            // search target node
            while (current->getNext() != nullptr) {
                if (current->getNext()->getUserID() == _userID && current->getNext()->getItemID() == _itemID){
                    current->setNext(current->getNext()->getNext());
                    if (_print) cout << "Customer rating (" << _userID << ", " << _itemID << ") is removed successful" << endl;
                    return;
                }
                current = current->getNext();
            }
            // couldn't find target
            if (_print) cout << "Customer rating (" << _userID << ", " << _itemID << ") does not exist" << endl;
        }

        /*
            Returns rating value
            If not found, returns 0.0
        */
        double rating(int _userID, int _itemID, bool _print = true){

            if (debug) {
                if (head == nullptr) cout << "Debug: The list is empty, no ratings to check" << endl;
            }

            Node* current = head;
            while (current != nullptr) {
                if (debug) cout << "Debug: Checking node (User: " << current->getUserID() << ", Item: " << current->getItemID() << ", Value: " << current->getValue() << ")" << endl;
                if (current->getUserID() == _userID && current->getItemID() == _itemID) {
                    double _value = current->getValue();
                    if (_print) cout << "Customer rating (" << _userID << ", " << _itemID << ") is: " << _value << endl;
                    if (debug) cout << "Debug: Customer rating (User: " << _userID << ", Item: " << _itemID << ", Value: " << _value << ") is found, returns " << _value << endl;
                    return _value;
                }
                current = current->getNext();
            }
            if (_print) cout << "Customer rating (" << _userID << ", " << _itemID << ") is: 0.0" << endl;
            if (debug) cout << "Debug: Customer rating (User: " << _userID << ", Item: " << _itemID << ", Value: " << 0.0 << ") is not found, returns 0.0" << endl;
            return 0.0;
        }


        /*
            Returns the average rating value
            If no items, returns 0.0
        */
        double average (int _itemID, bool _print = true) {

            Node *current = head;
            double sum = 0;
            int count = 0;
            while (current != nullptr) {
                if (current->getItemID() == _itemID) {
                    sum += current->getValue();
                    count++;
                }
                current = current->getNext();
            }
            double average;
            if (count == 0) { average = 0.0; }
            else { average = sum / count; }

            if (_print) cout << "Average rating (" << _itemID << ") is: " << average << endl;
            return average;
        }
};


void command(LinkedList& _list, string _input) {

    string commandArray[4];

    // convert input into string array via stringstream
    stringstream ssin(_input);

    int i = 0;
    while (ssin.good() && i < 4) {
        ssin >> commandArray[i];
        ++i;
    }

    // commands

    if (commandArray[0] == "INSERT") {
        int userID = stoi(commandArray[1]);
        int itemID = stoi(commandArray[2]);
        double value = stod(commandArray[3]);
        _list.insert(userID, itemID, value);
    }
     else if (commandArray[0] == "REMOVE") {
        int userID = stoi(commandArray[1]);
        int itemID = stoi(commandArray[2]);
        _list.remove(userID, itemID);
    }
     else if (commandArray[0] == "RATING") {
        int userID = stoi(commandArray[1]);
        int itemID = stoi(commandArray[2]);
        _list.rating(userID, itemID);
    }
     else if (commandArray[0] == "AVERAGE") {
        int itemID = stoi(commandArray[1]);
        _list.average(itemID);
    }
     else {
        cout << "Invalid command" << endl;
    }
}

int main() {

    LinkedList list;
    
    while (true) {
        string input;
        getline(cin, input);
        command(list, input);
    }

    return 0;
}