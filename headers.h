#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>


// Estrutura de um registro
typedef struct rec {
	
	bool valid;
	int codigoLivro; // Primary key
	char titulo[30];
	char nomeCompletoPrimeiroAutor[10];
	int anoPublicacao;
	
} recordNode;


// Estrutura de um node da arvoreB
typedef struct bTreeNode {

	bool valid;
	int noOfRecs;
	bool isLeaf;
	int pos;
	int keyRecArr[2*t - 1]; // As keys do node, max: 2t-1
	int posRecArr[2*t - 1]; // Posições dos registros no arquivo data.dat, max: 2t-1
	int children[2*t];

} bTreeNode;


typedef struct tree {

	char fileName[20];
	FILE* fp;
	int root;
	int nextPos;

} bTree;


bTree* createTree(char* fileName,bool mode);
bTreeNode* nodeInit(bTreeNode* node,bool isLeaf,bTree* tree);
void insert(bTree* tree,recordNode* record);
void delete(bTree* tree,int key);
void traverse(bTree* tree, int root);
void dispNode(bTreeNode* node);
void writeFile(bTree* ptr_tree, bTreeNode* p, int pos);
void readFile(bTree* ptr_tree, bTreeNode* p, int pos);


void enterData(recordNode* record, int id_num, char country[], char Grate[], int Score, int Rate);
recordNode* getData(char *filepath, int len);
recordNode* search(bTree* tree, int key);
recordNode* searchRecursive(bTree* tree, int key, bTreeNode* root);
bool removeFromTree(bTree* tree, int key);
bTreeNode* merge(bTree* tree, bTreeNode *node, int idx);
void borrowFromNext(bTree* tree, bTreeNode *node, int idx);
void borrowFromPrev(bTree* tree, bTreeNode *node, int idx);
void fill(bTree* tree, bTreeNode *node, int idx);
recordNode *getSucc(bTree* tree, bTreeNode *node, int idx);
recordNode *getPred(bTree* tree, bTreeNode *node, int idx);
void removeFromNonLeaf(bTree* tree, bTreeNode *node, int idx);
void removeFromLeaf (bTree* tree, bTreeNode *node, int idx);
void removeNode(bTree* tree, bTreeNode* node, int k);
int findKey(bTreeNode* node, int k);
