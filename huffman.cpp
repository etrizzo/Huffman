//
//  huffman.cpp
//  Huffman encoder/decoder which can read and compress text files using the Huffman algorithm.
//	Can decompressed files compressed using the format specified by this program.
//
//  Created by Emily Rizzo on 3/11/17.
//  Copyright © 2017 Emily Rizzo. All rights reserved.
//
#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <stack>
#include <sstream>
#include <fstream>
#include <random>
#include <stdlib.h>
#include <iomanip>
#include <time.h>
using namespace std;


class Node;
class Tree;
bool debug = false;
bool byFreq = true;

string TXT_EXT = ".txt";
string HUF_EXT = "-HUF.txt";
string OUT_EXT = "-o.txt";

string HUF_DIR = "HUF-files/";
string OUT_DIR = "OUT-files/";
string TXT_DIR = "TXT-files/";

//returns the character associated with a string representing 8 bits (i.e. "00110000" (48) returns '0')
unsigned char byteToChar(string byte){
    unsigned char c = 0;
    int i = 0;
    while (i < byte.size()){
        unsigned char bit = byte[i] - '0';
        c = c << 1 | bit;
        i++;
    }
    return c;
}

//returns a string representing the 8 bits associated with char c (i.e. '0' returns "00110000")
string charToByte(unsigned char c){
    string byte;
    int cnum = (int) c;
    while (cnum > 0){
        byte = to_string(cnum%2) + byte;
        cnum= (int) cnum / 2;
    }
    while (byte.size() < 8){
        byte = '0' + byte;
    }
    return byte;
}

Tree decodeTree(string encoding);


class Node{
public:
    string name;    //the chars represented
    string printedName;     //printed representation of chars for toString (i.e. \n char prints out "\n")
    string code;    //the path to the char
    int frequency;
    bool isLeaf;
    Node* left;
    Node* right;
    Node(string n);
    Node(Node* l, Node* r);		//constructor of an internal leaf - l and r are the node's children.
    
    // prints out node in format of "  leaf  ||  freq  ||  code
    void toString(){
        if (name.size() < 5){
            cout << setfill(' ') << setw(5) << printedName;
        } else {
            cout << printedName;
        }
        cout << " || ";
        cout <<setfill(' ') << setw(8) << to_string(frequency);
        cout << " || ";
        cout << code;
    }
    
    // postOrder traversal of tree that constructs a list of nodes in order of visitation,
    // and generates bit codes for each node
    vector<Node *> postOrder(string c, vector<Node *> list){
        code = c;
        if (!isLeaf){
            if (left != NULL){
                list = left->postOrder(c + "0", list);
            }
            if (right != NULL){
                list = right->postOrder(c + "1", list);
            }
        }
        list.push_back(this);
        return list;
    }
    
    //used to call recursive postOrder() with default args
    vector<Node *> postOrder(){
        vector<Node *> l;
        return postOrder("", l);
    }
    
    //converts common characters to their code representations, for tree printing
    //i.e. "\n" newline character will print out the string "\n" in table instead of a new line
    string printedString(string n){
        switch(n[0]){
            case '\n': return "\\n";
            case ' ': return "\' \'";
            case '\t': return "\\t";
            case '\r': return "\\r";
            case '\0': return "\\0";
            case '\275': return "\\275";
            case '\277': return "\\277";
            case '\357': return "\\357";
        }
        return n;
    }
};
    
    //constructor for creating a leaf node
    Node::Node(string n){
        name = n;
        printedName = printedString(n);
        isLeaf = true;
        frequency = 0;
    }
    
    //constructor for creating an internal node, l and r are the node's 2 children
    Node::Node(Node* l, Node* r){
        name = l->name + " " + r->name;
        printedName = name;
        left = l;
        right = r;
        frequency = left->frequency + right->frequency;
        isLeaf = false;
    }
    
    //comparison for sorting nodes by frequency
    bool compareNodes(Node *i, Node *j){
        return (i->frequency < j->frequency);
    }
    
    // Tree structure, holds the list of all leaves in the tree (freqs)
    // and constructs the tree using the huffList vector
    class Tree{
    public:
        vector<Node*> freqs;
        vector<Node*> huffList;
        Node * root;
        
        
        //returns the node if it is in the tree, or adds and returns a new node if not
        Node* contains(string name){
            for(int i = 0; i < freqs.size(); i++){
                if (freqs[i]->name == name){
                    return freqs[i];
                }
            }
            Node* ret = new Node(name);
            freqs.push_back(ret);
            huffList.push_back(ret);
            return ret;
        }
        
        //returns char associated with bit path, or "" if does not exist
        string containsCode(string code){
            for (int i = 0; i < freqs.size(); i++){
                if (freqs[i]->code == code){
                    return freqs[i]->name;
                }
            }
            return "";
        }
        
        //Adds a character to the frequency list. Either creates a new node if char is not in tree, or retreives existig node.
        void addChar (string name){
            Node* n = contains(name);
            n->frequency++;
        }
        
        //prints the current tree in the format:
        //		"  leaf  ||  freq  ||   code  "
        void printTree(){
            int size = (int) freqs.size();
            cout << " leaf ||   freq   ||    code \n";
            cout << "================================\n";
            for (int i = 0; i < size; i++){
                freqs[i]->toString();
                cout << "\n";
            }
        }
        
        //sorts by frequency, then prints the current tree
        void sortTree(){
            sort(freqs.begin(), freqs.end(), compareNodes);
            printTree();
        }
        
        //debugging method for printing out the huffList used to construct tree
        void printhuffList(){
            int size = (int) huffList.size();
            cout <<"\nHUFFLIST:\n";
            cout << " leaf ||   freq   ||    code \n";
            cout << "================================\n";
            for (int i = 0; i < size; i++){
                huffList[i]->toString();
                cout << "\n";
            }
            cout <<"\n";
        }
        
        //inserts node n into hufflist, by frequency
        void insertByFreq(Node* n){
            long i = 0;
            long freq = n->frequency;
            long size = huffList.size();
//            printhuffList();
            while ((i < size) && (freq > huffList[i]->frequency)){
                i++;
            }
            huffList.insert(huffList.begin()+i, n);
            if (debug){
                printhuffList();
            }
        }
        
        //constructs a huffman tree using the huffList, which contains all leaves added to freqs
        void Huffman(){
            sort(huffList.begin(), huffList.end(), compareNodes);
            while (huffList.size() > 1){
                // Combine 2 lowest frequency nodes into 1 internal node (i.e. {A:1} and {B:1} become {A B:2}
                insertByFreq(new Node(huffList[0], huffList[1]));		//insert frequency in order to avoid having to re-sort
                huffList.erase(huffList.begin(), huffList.begin()+2);		//remove the 2 lowest frequency nodes
            }
            vector<Node *> order;
            root = huffList[0];
            postOrder();
            if (debug){				//if debugging, print out tree when it is created
                printTree();
            }
        }
        
        //performs a post order traversal on the root and returns the list of nodes
        vector <Node *> postOrder(){
            return root->postOrder();
        }
        
        //encodes the tree and returns the string representation of bits
        string Encode(){
            //counts number of leaves in the tree, for decoding purposes
            //encode 0 for internal nodes
            //encode 1 for leaves, followed by 8 bytes for char (EOF: 11111111);
            vector<Node *> traversal = postOrder();
            int numLeaves = 0;
            string code;
            for (int i = 0; i<traversal.size(); i++){
                string byte;
                string n = traversal[i]->name;
                if (traversal[i]->isLeaf){		//if the traversal is a leaf, write a "1" followed by the 8 bits for the character at that leaf
                    numLeaves++;
                    code += "1";
                    if (n != "EOF"){			//if the character is EOF, assign byte 11111111
                        byte = charToByte((unsigned char)n[0]);
                    } else {
                        byte = "11111111";
                    }
                    code += byte;
                } else {            //else write a "0", signifying internal node
                    code+="0";
                }
            }
            unsigned char leaves = numLeaves;
            string leafnum = charToByte(leaves);	//get 1 byte representing the number of leaves
            code = leafnum + code;      //prepend the number of leaves to the encoding;
            
            return code;
        }
        
        void removeTree(){
            //free mem allocated to tree
            int freqSize = (int)freqs.size();
            if (freqSize > 0){
                for(int i=0 ; i < freqs.size() ; i++)
                    delete freqs[i];
                freqs.clear();
            }
            if (huffList.size() != freqSize){
                for (int i=0; i < huffList.size();i++)
                    delete huffList[i];
                huffList.clear();
            }
            
        }
        
    };
    
    //removes the extension from a file name to get the root of the file
    string removeExt(string fileName, int size = -1){
        if (size == -1){
            size = (int) fileName.size();
        }
        string s;
        int i = 0;
        while (fileName[i] != '.' && i < size){
            s += fileName[i];
            i++;
        }
        return s;
    }
    
    //separate a string of bits into bytes, for readability in debugging
    void sepEight(string bits){
        for (int i = 0; i < bits.size(); i ++){
            cout << bits[i];
            if (i % 8 == 7){
                cout << " ";
            }
        }
        cout << "\n";
    }
    
    
    
    //decodes a string representation of bits by
    //	 1. decoding a huffman tree from the start of the file
    //	 2. decoding the remainder of the file based on the tree generated
    string decodeBytes (string bytes){
        string bitCode;
        string decoded;
        Tree t;
        
        t = decodeTree(bytes);				// generates huffman tree from the file
        string enc = t.Encode();			// used to determine the size of the encoded tree, to determine where to start decoding chars
        if (debug){
            cout << "Tree encoded is size: " << enc.size() << "\n";
        }
        
        //begin decoding bytes immediately after the encoded tree
        for (int i = (int)enc.size(); i < bytes.size(); i++){
            bitCode+= bytes[i];
            string inTree = t.containsCode(bitCode);		//get the character associated with the current bit code
            if (inTree != ""){				//if the bit code is in the tree, add the character to the decoded string
                if (inTree == "EOF"){				//and reset the current bitCode.
                    break;							//Stop decoding when the EOF char is encountered.
                }
                decoded+=inTree;
                bitCode = "";
            }
        }
        t.removeTree();
        return decoded;
    }
    
    
    //returns the next 8 chars of a string code (one byte), starting at int start
    string nextByte(int start, string code){
        string byte;
        for (int i = start; i < start+8;i++){
            byte+=code[i];
        }
        return byte;
    }
    
    
    //decodes a huffman tree at the start of the binary string "encoding" of a compressed file.
    //encoding should have the following format:
    //<single byte representing number of leaves in the huffman tree> <binary encoding of the huffman tree> <binary encoding of the file according to huffman bits>
    Tree decodeTree(string encoding){
        Tree t;
        stack<Node *> s;		//stack used for creating tree
        
        string byte;
        unsigned char c;
        string nodeName;
        
        int i = 0;				//index of the encoding
        string leafByte = nextByte(i, encoding);		//get the byte representing te number of leaves in the encoding
        i+=8;			//increment index, calling nextByte moves forward 8
        unsigned char leafChars = byteToChar(leafByte);
        int numLeaves = (int) leafChars;			//number of leaves is used to determine when the entire tree has been read
        int leavesFound = 0;
        
        
        //while the tree has not been fully read, decode
        while ((leavesFound < numLeaves) || s.size() > 1)  {		//while there are still leaves to be read, or there are still nodes in the stack, keep reading tree
            if (i > encoding.size()){		//if attempting to decode a file with an incorrect format, it should be impossible to decode a huffman tree, and an error should be thrown.
                throw invalid_argument( "\nCompressed file is not in the correct format. Could not decode huffman tree. \nRun \"encode <filename>\" to generate a file with the correct format.\n" );
            }
            if (encoding[i] == '1'){        //if a leaf is found, find next 8 bits, and create a leaf node
                i++;
                byte = nextByte(i,encoding);
                if (byte == "11111111"){		//if the EOF byte is found, assign name to be "EOF"
                    nodeName = "EOF";
                } else {						//otherwise, decode the byte to the character associated with the leaf
                    c =byteToChar(byte);
                    string x(1, static_cast<char>(c));
                    nodeName = x;
                }
                s.push(new Node(nodeName));
                t.freqs.push_back(s.top());
                i+=8;
                leavesFound++;
            } if (encoding[i] == '0') {     //if an internal node is found, pop off 2 nodes from stack and combine
                i++;
                Node *n1 = s.top();
                s.pop();
                Node *n2 = s.top();
                s.pop();
                s.push(new Node (n2, n1));			//push the new node onto the stack
            }
            
        }
        
        t.huffList.push_back(s.top());
        t.root = t.huffList[0];
        t.root->postOrder();         //assigns bit codes to leaves
        return t;
    }
    
    
    // encode the file "fileRoot.txt" to "fileRoot-HUF.txt" using the huffman tree t
    // Encoding format:
    // <single byte representing number of leaves in the huffman tree> <binary encoding of the huffman tree> <binary encoding of the file according to huffman bits>
    // returns the number of bytes in the encoded file
    long encode(string fileRoot, Tree t){
        long encodedSize = 0;
        char c;
        string fileBits;        //the string representation of the bits to be written to encoded file
        Node *n;
        
        string newFile = HUF_DIR + fileRoot + HUF_EXT;
        ifstream file;
        file.open(TXT_DIR + fileRoot + TXT_EXT);
        if (!file.is_open()){
            return -1;
        }
        ofstream output;
        output.open(newFile, ios::binary | ios::trunc);
        
        string encodedTree = t.Encode();
        fileBits+=encodedTree;      //adds the encoded tree (w/ number of leaves) to the front of the encoded file
        
        while (file.get(c)){		//read through the file, and encode each character
            string s(1,c);
            n = t.contains(s);       //gets the character's node w/ frequency and path
            string path = n->code;
            fileBits+=n->code;
        }
        
        n = t.contains("EOF");
        fileBits+=n->code;			//add an EOF character to the end of the file, to signify when decoding should stop (in case of buffer bits at the end of the file)
        
        while (fileBits.size() % 8 != 0){		//pad the final byte with 0's
            fileBits+='0';
        }
        
        char outc;
        string byte;
        for (int i = 0; i < fileBits.size(); i++){			//write each byte in fileBits to output file as a char
            byte +=fileBits[i];
            if (i % 8 == 7){		//when an entire byte is read, write the char to output file
                encodedSize++;		//increment number of bytes in output file
                outc = byteToChar(byte);
                byte = "";
                if (output.is_open()){
                    output << outc;
                }
            }
        }
        
        file.close();
        output.close();
        
        return encodedSize;
        
    }
    
    
    // decodes the file "fileRoot-HUF.txt" to the file "fileRoot-o.txt"
    // -huf files must be of the following format:
    // <single byte representing number of leaves in the huffman tree> <binary encoding of the huffman tree> <binary encoding of the file according to huffman bits>
    // returns the number of bytes in the decoded file
    long decode(string fileRoot){
        float start;
        float finish;
        float diff;
        if (debug){     //for timing
            start = clock();
        }
        ifstream file;
        file.open(HUF_DIR + fileRoot + HUF_EXT);
        if (!file.is_open()){
            return -1;
        }
        
        string newFile = OUT_DIR + fileRoot + OUT_EXT;
        ofstream output;
        output.open(newFile, ios::trunc);
        
        char csigned;
        string bytes;					//string representing all bits in the file
        while (file.get(csigned)){		//construct string bytes by decoding each character in the file
            unsigned char c = (unsigned) csigned;
            string byte = charToByte(c);
            bytes+=byte;
        }
        
        //decode the string of bytes and write to output
        string x = decodeBytes(bytes);
        output << x;
        output.close();
        if (debug){
            finish = clock();
            diff = finish - start;
            cout << "Decoded in: " << diff << " seconds\n";
        }
        
        return x.size();
        
    }
    
    //constructs and returns a huffman tree for file fileName
    Tree  makeTree(string fileName){
        ifstream file(fileName);
        vector<Node *> freqs;
        Tree t;
        char c;
        cout << c;
        while (file.get(c)){		//add a node for each character in the file
            string s(1, c);
            t.addChar(s);			//addNode adds a new node if node is not in the tree, or increments frequency of existing node
        }
        t.addChar("EOF");			//add the EOF character to the tree, used to signify end of encoding
        cout <<"\n";
        t.Huffman();
        file.close();
        
        return t;
    }

    //writes one million e's to a file
    void writeE(){
        ofstream output;
        string fileName = TXT_DIR + "eee" + TXT_EXT;
        output.open(fileName, ios::trunc);
        for (int i = 0; i < 1000000; i++){
            output << "e";
        }
    }

    //writes file of random (non-space) ASCII characters (i.e. no new lines, no tabs)
    void writeRandom(){
        ofstream output;
        string fileName =TXT_DIR + "random" + TXT_EXT;
        output.open(fileName, ios::trunc);
        for (int i = 0; i < 1000000; i++){
            char c = (char) (rand() % (127-33) + 33);
            output << c;
        }
    }
    
    //splits the first word (the command) off of a string, and returns a vector [<command>, <rest of input>]
    vector<string> processCommand(string input){
        string command;
        string remainder;
        char delim = ' ';
        bool split = false;
        for (int i = 0; i < input.size(); i++){
            char c = input[i];
            if (c == delim){
                split = true;
            } else {
                if (split){
                    remainder+=c;
                } else {
                    command+=c;
                }
            }
        }
        vector<string> r;
        r.push_back(command);
        r.push_back(remainder);
        return r;
    }

    //prints out command prompt
    vector<string> getCommand(){
        string command;
        cout << "Commands: \n";
        cout << "\"run <filename>\", \"encode <filename>\", \"decode <filename>\", \"print\", \"exit\"";
        cout << "\n\nEnter a command (type \"help\" for info): ";
        getline(cin, command);
        return processCommand(command);
    }

    void printFiles(){
        cout << "Choose a file to run through the Huffman encoder from the list below:\n\n";
        cout << "file.txt               //\"go go gophers\"\n";
        cout << "lipsum.txt             //A single paragraph of Lorem Ipsum text\n";
        cout << "lipsum-long.txt        //5 paragraphs of Lorem Ipsum text\n";
        cout << "lipsum-longest.txt     //20 paragraphs of Lorem Ipsum text\n";
        cout << "hamlet.txt             //\"Hamlet\" by William Shakespeare\n";
        cout << "cities.txt             //\"A Tale of Two Cities\" by Charles Dickens\n";
        cout << "eee.txt                //just 1 million e's\n";
        cout << "random.txt             //1 million randomly generated letters (slow)\n";
        
        cout << "\n";
        cout << "Enter \"run <filename>\" to encode/decode a file.\n";
        cout << "additional files should be added to directory /TXT-files (will not appear in this list)\n";
    }

    void printHelp(){
        cout << "Files will be encoded with the extension \""<< HUF_EXT <<"\", and decoded with the extension \"" << OUT_EXT <<"\"\n";
        cout << "Commands:\n";
        cout << "run <filename>         //Runs Encode and Decode on <filename>\n";
        cout << "encode <filename>      //Encodes <filename> to <filename>" << HUF_EXT << "\n";
        cout << "decode <filename>      //Decodes <filename> (with extension: " << HUF_EXT << ") to <filename>" << OUT_EXT <<"\n";
        cout << "print                  //prints the last encoded file's huffman tree, with frequencies and bit codes\n";
        cout << "files                  //print the list of default files available for encoding\n";
        cout << "help                   //print this list of commands and descriptions\n";
        cout << "exit                   //exit the program\n";
        cout << "\n";
        cout << "All commands can be shortened to their first letter\n";
        cout << "(i.e. \"p\" = \"print\", \"r <filename>\" = \"run <filename>\")\n\n";
        
    }
    
    
    int main(int argc, const char * argv[]) {
        Tree t;
        t = makeTree("lipsum-longest.txt");
        string command;
        string input;
        vector<string> splitCommand;
        printFiles();
        cout <<"\n";
        splitCommand = getCommand();
        command = splitCommand[0];
        input = splitCommand[1];
        string fileRoot;
        while (command != "exit"){
            if (command == "help" || command == "h"){
                printHelp();
            } else if (command == "files" || command == "f"){
                printFiles();
            }else if (command == "print" || command == "p"){
                cout << "\nprinting sorted tree for: " << fileRoot <<"\n";
                t.sortTree();
            } else if (command == "encode" || command == "e"){
                fileRoot = removeExt(input);
                if (fileRoot == "random"){
                    cout << "writing a random file...\n";
                    writeRandom();
                }
                t = makeTree(TXT_DIR + fileRoot + TXT_EXT);
                long eSize = encode(fileRoot, t);
                if (eSize == -1){
                    cout << "\nERROR: Could not find file "<< input<< "\n";
                } else {
                    cout << "encoded file " << fileRoot << TXT_EXT << " to " << fileRoot << HUF_EXT << "\n\n";
                    cout << fileRoot << HUF_EXT << ":       " << eSize << " bytes\n";
                }
            } else if (command == "decode" || command == "d"){
                int location = (int)input.find(HUF_EXT);
                if ((location != -1) && (location == (input.size() - HUF_EXT.size()))){
                    fileRoot = removeExt(input, location);
                    long dSize = decode(fileRoot);
                    if (dSize == -1){
                        cout << "\nERROR: Could not find file "<< input<< "\n";
                    } else {
                        cout << "decoded file " << fileRoot << HUF_EXT << " to " << fileRoot << OUT_EXT << "\n\n";
                        cout << fileRoot << OUT_EXT << ":           " << dSize <<" bytes\n";
                    }
                } else {
                    cout << "\nERROR: " << input << " is not a valid huffman encoded file for this program\n";
                    cout << "Please run \"encode\" <filename> to create a valid huffman encoded file\n";
                }
            }else if (command == "run" || command == "r"){
                fileRoot = removeExt(input);
                if (fileRoot == "random"){
                    cout << "writing a random file...\n";
                    writeRandom();
                }
                t = makeTree(TXT_DIR + fileRoot + TXT_EXT);
                long eSize = encode(fileRoot, t);
                if (eSize == -1){
                    cout << "ERROR: Could not find file "<< input<< "\n";
                } else {
                    cout << "encoded file " << fileRoot << TXT_EXT << " to " << fileRoot << HUF_EXT << "\n";
                
                    long dSize = decode(fileRoot);
                    if (dSize == -1){
                        cout << "ERROR: Could not find file "<< input << "\n";
                    } else {
                        cout << "decoded file " << fileRoot << HUF_EXT << " to " << fileRoot << OUT_EXT << "\n\n";
                    
                        cout << fileRoot << TXT_EXT << ":           " << dSize <<" bytes\n";
                        cout << fileRoot << HUF_EXT << ":       " << eSize << " bytes\n\n";
                        float percent = ((float)eSize / (float)dSize) * 100.0;
                        cout << fileRoot << HUF_EXT << " is %" << percent << " the size of " << fileRoot << TXT_EXT << "\n";
                        
                        
                        if (dSize < eSize){
                            cout << "  Note: small files may have larger huffman encodings because the binary tree must be stored in the file\n";
                        }
                    }
                }
            } else {
                cout << "\nERROR: " << command << " is not a valid command. Please enter \"help\" for a list of valid commands\n";
            }
            
            cout << setfill('=') << setw(33) << "\n";
            //get the next commmand
            splitCommand = getCommand();
            command = splitCommand[0];
            input = splitCommand[1];
        }
        
        
        t.removeTree();
        splitCommand.clear();
        
        return 0;
    }
