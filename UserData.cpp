#include "UserData.h"
#include <vector>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <fstream>

/*UserData() constructor: builds root with "null" username and initializes
variables for later use.*/
UserData::UserData()
{
    userText="";
    root=new User;
    root->username="null"; //when adding users, always check if name is null before adding and if so, refuse it
    root->parent=NULL;
    root->leftChild=NULL;
    root->rightChild=NULL;
}

UserData::~UserData(){} //destructor does nothing

/*void interface(string); is called from main to access listUsers and
exporter methods. These functions are recursive and require the tree's
root be fed in, thus making them uncallable from main.  Always called
with string arguments "list" or "export", each corresponding to the
function it accesses. private string exportLine is initially blank, but
after calling exporter, all user data is written to it, which the interface
method then writes to logins.txt*/
void UserData::interface(string occasion){ //this won't count as one of the 10 methods, too cheaty
    if(occasion=="list"){
        listUsers(root);
    }
    else if(occasion=="export"){
        ofstream outFile("logins.txt");
        exporter(root);
        outFile<<exportLine;
        outFile.close();
    }
}

/*void importer(); is immediately run in main when the program starts.
It reads a text file of the form "user,pass\n" and passes in those usernames
and passwords to addUser, which actually builds the tree*/
void UserData::importer()
{
    ifstream inFile("logins.txt");
    string importUser,importPass;
    while(inFile){
        getline(inFile,importUser,',');
        getline(inFile,importPass,'\n');
        if(importUser != ""){ //last line is there, but empty - program mistakenly processes it as user. this catches that
            addUser(importUser, importPass);
        }
    }
}

/*void exporter(User*); is indirectly called through the 'interface' method and
begins a recursive inorder traversal of the tree. It writes userdata to string
exportLine, which the interface function then writes to logins.txt.  It initially
takes the root of the tree as an argument.*/
void UserData::exporter(User* x)
{
    if(x->leftChild != NULL){
        exporter(x->leftChild);
    }
    if(x->username != "null"){ //skips over 'null' root
        exportLine=exportLine+x->username+","+x->password+"\n"; ///seems like this will get astoundingly inefficient with large trees
    }
    if(x->rightChild != NULL){
        exporter(x->rightChild);
    }
}

/*string login(string,string); is called when the user tries to login and
takes a string username and password that the user enters.  It searches the
tree and returns one of three possibilities: a) a node with the right username
was found and the entered password matches, b) a node with the right username
was found, but the password was wrong, and c) the user wasn't found.  Upon
a successful login, it sets global sessionUser and sessionPassword variables
for use in several other methods.*/
string UserData::login(string username, string password)
{
    User* checker;
    checker=searcher(username); //searcher just returns closest match
    if(checker->username==username && checker->password==password){
        sessionUser=username; //sets active user for later use
        sessionPassword=password;
        return "true";
    }
    else if(checker->username==username && checker->password != password){
        return "badPass";
    }
    else{
        return "badUser"; //returns string so that we can have these three different returns; 'true' and 'false' aren't enough
    }
}

/*bool addUser(string,string); is called when the user wants to add a new
account.  It takes the username and password they are requesting to use
and first checks if the username is already taken, and if not, it adds the user
to the tree.  addUser is also called from the importer method, once for each
user/pass pair it finds in logins.txt.*/
bool UserData::addUser(string username, string password)
{
    User *test;
    test=searcher(username);
    if(test->username==username){ //user already exists
        return false;
    }
    User *newUser=new User; //only gets this far if user doesn't already exist
    newUser->username=username;
    newUser->password=password;
    newUser->leftChild=NULL;
    newUser->rightChild=NULL;
    User *x=root;
    User *parent=NULL;
    while(x != NULL){
        parent=x;
        if(newUser->username.compare(x->username)<0){
            x=x->leftChild;
        }
        else{
            x=x->rightChild;
        }
    }
    newUser->parent=parent;
    if(newUser->username.compare(parent->username)<0){
        parent->leftChild=newUser;
    }
    else{
        parent->rightChild=newUser;
    }
    return true;
}

/*void listUsers(User*); is called from the interface function with the
tree's root as an argument.  It begins an inorder, recursive readout of
the tree. It changes nothing and the user can repeatedly call it without
changing anything.*/
void UserData::listUsers(User* x)
{
    if(x->leftChild != NULL){
        listUsers(x->leftChild);
    }
    if(x->username != "null"){ //skips over 'null' root
        cout<<x->username<<endl;
    }
    if(x->rightChild != NULL){
        listUsers(x->rightChild);
    }
}

/*bool changePass(string,string); is called once the user is logged in and can
be used to change their account's password (their old password being stored
in sessionPassword, set in the login method) and takes their old password and
the new one.  main() asks for the old password as a security measure and if it
doesn't equal sessionPassword, it refuses the password change and returns false.
If it does, it finds the user in the tree and sets their password as argument
newPass and returns true.*/
bool UserData::changePass(string oldPass, string newPass)
{
    if(oldPass != sessionPassword){
        return false;
    }
    else{
        User *findUser;
        findUser=searcher(sessionUser); //user already logged in, so findUser should ALWAYS get the right user, not just the closest
        findUser->password=newPass;
        return true;
    }
}

/*void deleteAccount() can only be called by the user once they're logged in,
takes no arguments, and returns nothing.  It finds the node with username
sessionUser (set upon successful login) and then deletes that node. There are
three conditions: the node has no children, one child, or two children. After
a successful deletion and the program returns to main(), the user is returned
to the main menu and asked to login, create an account, or quit.*/
void UserData::deleteAccount()
{
    User* x=searcher(sessionUser);
    //x->username definitely equals sessionUser
    if(x->leftChild==NULL && x->rightChild==NULL){ ///NO CHILDREN
        if(x->parent->leftChild==x){
            x->parent->leftChild=NULL;
        }
        else{
            x->parent->rightChild=NULL;
        }
        delete x;
    }
    else if((x->leftChild==NULL && x->rightChild != NULL) || (x->leftChild != NULL && x->rightChild==NULL)){ ///ONE CHILD
        if(x->leftChild==NULL && x->rightChild != NULL){
            if(x->parent->leftChild==x){
                x->parent->leftChild=x->rightChild;
                x->rightChild->parent=x->parent;
                delete x;
            }
            else{
                x->parent->rightChild=x->rightChild;
                x->rightChild->parent=x->parent;
                delete x;
            }

        }
        else{
            if(x->parent->leftChild==x){
                x->parent->leftChild=x->leftChild;
                x->leftChild->parent=x->parent;
                delete x;
            }
            else{
                x->parent->rightChild=x->leftChild;
                x->leftChild->parent=x->parent;
                delete x;
            }
        }
    }
    else if(x->leftChild != NULL && x->rightChild != NULL){ ///TWO CHILDREN
        User* xRight=x->rightChild;
        if(xRight->leftChild==NULL && xRight->rightChild==NULL){
            x->username=xRight->username;
            x->password=xRight->password;
            delete xRight;
            x->rightChild=NULL;
        }
        else{
            if((x->rightChild)->leftChild != NULL){
                User* xLeft=(x->rightChild)->leftChild;
                User* xLeftP=(x->rightChild);
                while(xLeft->leftChild != NULL){
                    xLeftP=xLeft;
                    xLeft=xLeft->leftChild;
                }
                x->username=xLeft->username;
                x->password=xLeft->password;
                delete xLeft;
                xLeftP->leftChild=NULL;
            }
            else{ //(x->right)->right != NULL
                User* temp=x->rightChild;
                x->username=temp->username;
                x->password=temp->password;
                x->rightChild=temp->rightChild;
                delete temp;
            }
        }
    }
}

/*void loadText(string); is called when the logged-in user asks to enter a
string of text, which loadText takes as argument 'text', splits into individual
words, and pushes into 'text' and 'textTemp', globally declared vectors of
textVector structs. It returns nothing, but filling text, textTemp, and textAlpha
makes it available for use in commonWords(), avgWordLength(), and sortText().
loadText clears the three vectors in the case that it's being called for the second
time, meaning the vectors may have contents from a previous analysis.*/
void UserData::loadText(string uText)
{
    text.clear(); //clear if user is doing this for a second time (ergo text will already have stuff in it)
    textTemp.clear();
    textAlpha.clear();
    userText=uText; //no need to reset userText to "", though
    istringstream is(uText);
    string word;
    textVector *newText=new textVector; //new word to be inserted
    while(is>>word){
        newText->word=word;
        newText->usages=1;
        text.push_back(*newText);
        textTemp.push_back(*newText); //for use in commonWords
        textAlpha.push_back(word); //for use in sortText
    }
    for(int i=0; i<text.size(); i++){ //calculates the usage of each text[i].word and inserts usage into text[i].usages
        for(int j=0; j<text.size(); j++){
            if(j!=i && text[i].word==text[j].word){
                text[i].usages++;
                textTemp[i].usages++; //for use in commonWords
            }
        }
    }
}

/*void commonWords(); takes no arguments.  It takes the now-filled textTemp,
removes repeats, and then: if textTemp.size()<10, prints words in order of
decreasing usage, or if textTemp.size()>10, prints the top 10 most common
words in order of decreasing usage.*/
void UserData::commonWords()
{
    if(userText==""){
        cout<<"No text detected."<<endl;
        return;
    }
    vectorSize=text.size();
    for(int i=0; i<textTemp.size(); i++){ //removes duplicate words
        for(int j=i+1; j<textTemp.size(); j++){
            if(textTemp[j].word==textTemp[i].word){
                textTemp[j].word="";
                textTemp[j].usages=-10000;
            }
        }
    }
    int wordsLeft=10;
    while(wordsLeft>0 && vectorSize>0){ //prints most common word, then second most common, etc, until textTemp is empty OR 10 most common words have been printed
        int maximum=0;
        int indexOfMax;
        for(int i=0; i<textTemp.size(); i++){
            if(textTemp[i].usages>maximum){
                maximum=textTemp[i].usages;
                indexOfMax=i;
            }
        }
        if(textTemp[indexOfMax].usages != -10000){ //excludes deleted items with usages=-10000
            cout<<textTemp[indexOfMax].word<<" ("<<textTemp[indexOfMax].usages<<" usages"<<")"<<endl;
        }
        textTemp[indexOfMax].word=""; //removes word that was just printed
        textTemp[indexOfMax].usages=-10000;
        vectorSize--;
        wordsLeft--;
    }
    userText="";
}

/*void avgWordLength(); takes no arguments and calculates the average length
of the words in textVector.  If there are no words in textVector (i.e. the
user hasn't entered anything), avgWordLength will say that no text is detected.*/
void UserData::avgWordLength()
{
    if(userText==""){
        cout<<"No text detected."<<endl;
        return;
    }
    float letterSum=0;
    float wordCount=0;
    float avgWordLength;
    for(int i=0; i<text.size(); i++){
        letterSum=letterSum+text[i].word.size();
        wordCount=wordCount+1;
    }
    avgWordLength=letterSum/wordCount;
    cout<<"Average word length: "<<avgWordLength<<" letters."<<endl;
    userText=""; //resets userText to prevent avgWordLength being called again without new text
}

/*void sortText(); takes no arguments and sorts the words in textVector
alphabetically.  If nothing is found in textVector (i.e. the use hasn't
entered anything), sortText will say that no text is detected.*/
void UserData::sortText()
{
    if(userText==""){
        cout<<"No text detected."<<endl;
        return;
    }
    vectorSize=text.size();
    bool flag;
    do{
        flag=0;
        for(int i=0; i<(vectorSize-1); i++){
            if(textAlpha[i]>textAlpha[i+1]){
                textAlpha[i].swap(textAlpha[i+1]);
                flag=1;
            }
        }
    } while(flag==1);

    for(int i=0; i<textAlpha.size(); i++){
        cout<<textAlpha[i]<<endl;
    }

    userText="";
}

/*User* searcher(string); is never called from main(), only from other
methods in the UserData class (login, addUser, changePass, deleteAccount).
It begins a standard search through the tree and returns a) the root with
username "null" if a node with the argument 'username' isn't found, or b)
the node with the same username as argument 'username'.*/
User* UserData::searcher(string username)
{
    User *x=root;
    while(x != NULL && username != x->username){
        if(username.compare(x->username)<0){
            x=x->leftChild;
        }
        else{
            x=x->rightChild;
        }
    }
    if(x==NULL){
        return root;
    }
    else{
        return x;
    }
}
