#include "headers.h"


// Cria ou lê uma Arvore B a partir do nome do arquivo inicializando as variáveis da struct bTree
bTree* createTree(char* fileName, bool mode) {

	bTree* tree = (bTree*) malloc(sizeof(bTree));
  
	// Switch para criar um arquivo novo ou usar um arquivo existente (mode = false cria novo arquivo, mode = true le um arquivo)
	if (!mode) {

		strcpy(tree->fileName, fileName);
		tree->fp = fopen(fileName, "w");
		fclose(tree->fp);

		tree->root = 0;
		tree->nextPos = 0;
	}

	else {
		FILE *oldFile = fopen("tree.dat", "r");
		fread(tree, sizeof(bTree), 1, oldFile);
		fclose(oldFile);
	}

	tree->fp = fopen(fileName, "r+");
	return tree;
}


// Inicializa um novo nó ou uma folha de uma árvore
bTreeNode* nodeInit(bTreeNode* node, bool isLeaf, bTree* tree) {
  
	node->valid = true;
	node->isLeaf = isLeaf;
	node->noOfRecs = 0;
	node->pos = tree->nextPos;
	(tree->nextPos)++;

	// inicialmente esse nó não possui filhos
	for (int i = 0; i < 2*t; ++i) node->children[i] = -1;
  
	return node;
}


// Responsável por escrever um bTreeNode em um arquivo associado à uma árvore-b. Pode receber uma posição específica que o Nó deve ser escrito
void writeFile(bTree* ptr_tree, bTreeNode* p, int pos) {

	// Verifica se o usuário não forneceu uma posição específica para escrever o nó no arquivo. Nesse caso, usa-se a próxima posição disponível
	if (pos == -1) pos = ptr_tree->nextPos++;

	// Se não, escreve na posição recebida
	fseek(ptr_tree->fp, pos * sizeof(bTreeNode), 0);
	fwrite(p, sizeof(bTreeNode), 1, ptr_tree->fp);
}


// Realiza o seek num nó de uma árvore de acordo com a posição e o tamanho de bTreeNode(bytes)
void readFile(bTree* ptr_tree, bTreeNode* p, int pos) {
  
	fseek(ptr_tree->fp, pos * sizeof(bTreeNode), SEEK_SET);
	fread(p, sizeof(bTreeNode), 1, ptr_tree->fp);
}


// Preenche um registro de um certo Nó
void enterData(recordNode* record, int codigoLivro, char titulo[], char nomeCompletoPrimeiroAutor[], int anoPublicacao) {

	record->valid = true;
	record->codigoLivro = codigoLivro;
	strcpy(record->titulo, titulo);
	strcpy(record->nomeCompletoPrimeiroAutor, nomeCompletoPrimeiroAutor);
	record->anoPublicacao = anoPublicacao;

	return;
}

// TEMP COMMENT: olhar porque "recordArr" ponteiro para registro, sendo q utilizaremos array de key e posicao dos registros
recordNode* getData(char* regdata, int len) {

	recordNode* recordArr = malloc(sizeof(recordNode) * len);
	char delim = ',';
	char line[256];
	int file_no = 0;
	int i, codigoLivro, anoPublicacao;
	char nomeCompletoPrimeiroAutor[255];
	char titulo[255];

	FILE* inpFile = fopen(regdata, "r");

 	while (file_no < len && fscanf(inpFile, "%d,%[^,],%[^,],%d,%d", &codigoLivro, titulo, nomeCompletoPrimeiroAutor, &anoPublicacao)) {
		enterData(&recordArr[file_no], codigoLivro, titulo, nomeCompletoPrimeiroAutor, anoPublicacao);
		file_no++;
	}

	return recordArr;
}


// Processo de consertar a arvore caso um Node esteja cheio na Insercao
void splitChild(bTree *tree, bTreeNode *x, int i, bTreeNode *y) {
	
	bTreeNode *z = malloc(sizeof(bTreeNode));
	nodeInit(z, y->isLeaf, tree);
	z->noOfRecs = t - 1;
	int j;
	
	// copia-se a segunda metade dos registros em "z", isso divide os registros igualmente entre "y" e "z"

	for (j = 0; j < t - 1; j++) z->posRecArr[j] = y->posRecArr[j + t];

	if (!y->isLeaf) {
	
		// se y não for uma folha atualiza-se os filhos de z e os de y.
		for (j = 0; j < t; j++) {
			z->children[j] = y->children[j + t];
			y->children[j + t] = -1;
		}
 	}
 	
	y->noOfRecs = t - 1;
	
	// processo de abertura de espaço para inserir o registro do meio de "y" no nó "x"

	for (j = (x->noOfRecs); j >= i + 1; j--) x->children[j + 1] = x->children[j];

	x->children[i + 1] = z->pos;

	for (j = (x->noOfRecs) - 1; j >= i; j--) x->posRecArr[j + 1] = x->posRecArr[j];

	x->posRecArr[i] = y->posRecArr[t - 1];
	x->noOfRecs++;

	writeFile(tree, x, x->pos);
	writeFile(tree, y, y->pos);
	writeFile(tree, z, z->pos);
	free(z);
}


// TEMP COMMENT: problema do recordArr
void insertNonFull(bTree* tree, bTreeNode* x, recordNode* record) {
	
	int i = (x->noOfRecs) - 1;
	
	if (x->isLeaf == true) {
	
		while ((i >= 0) && (record->codigoLivro < x->keyRecArr[i])) {
			x->posRecArr[i + 1] = x->posRecArr[i];
			i--;
		}
		
		x->posRecArr[i + 1] = record;
		(x->noOfRecs)++;

		writeFile(tree, x, x->pos);
	} 
  
	else {
		while ((i >= 0) && (record->codigoLivro < x->keyRecArr[i])) i = i - 1;
    
		bTreeNode *childAtPosi = malloc(sizeof(bTreeNode));
		readFile(tree, childAtPosi, x->children[i + 1]);

		if (childAtPosi->noOfRecs == (2 * t - 1)) {
			splitChild(tree, x, i + 1, childAtPosi);
			if (x->keyRecArr[i + 1] < record->codigoLivro) i++;
		}

		readFile(tree, childAtPosi, x->children[i + 1]);
		insertNonFull(tree, childAtPosi, record);

		free(childAtPosi);
	}
}


// Inserção balanceada em uma árvore. Para isso ela evita casos de lotamento de registros enquanto realiza a inserção de um registro
void insert(bTree *tree, recordNode *record) {
 
	// Se a árvore estiver vazia, insere o nó com o registro direto na raiz
	if (tree->nextPos == 0) {

		tree->root = tree->nextPos;

		bTreeNode *firstNode = malloc(sizeof(bTreeNode));
		nodeInit(firstNode, true, tree);
		firstNode->posRecArr[0] = record;
		(firstNode->noOfRecs)++;

		writeFile(tree, firstNode, firstNode->pos);

		free(firstNode);
		return;
	}
  
	// Se não, verifica se algum nó está cheio para ajudar em inserções posteriores, caso esteja, ocorre "split"
	else {
		bTreeNode *rootCopy = malloc(sizeof(bTreeNode));
		readFile(tree, rootCopy, tree->root);

		if (rootCopy->noOfRecs == (2 * t - 1)) {
		
			bTreeNode *newRoot = malloc(sizeof(bTreeNode));
			nodeInit(newRoot, false, tree);
			newRoot->children[0] = tree->root;

			splitChild(tree, newRoot, 0, rootCopy);

			int i = 0;
			if (newRoot->keyRecArr[0] < record->codigoLivro) i++;

			bTreeNode *childAtPosi = malloc(sizeof(bTreeNode));
			readFile(tree, childAtPosi, newRoot->children[i]);
			insertNonFull(tree, childAtPosi, record);

			tree->root = newRoot->pos;

			free(newRoot);
			free(childAtPosi);
		}
    
		// Caso não, apenas insere o registro no nó
		else insertNonFull(tree, rootCopy, record);
		free(rootCopy);
	}
}


// Chama o dispNode() para cada nó numa árvore, i.e. imprime informações de todos nós filhos a partir de um nó dado
void traverse(bTree *tree, int root) {

	if (root == -1) return;

	// Aloca o tamanho de um registro para a memória principal
	bTreeNode* toPrint = malloc(sizeof(bTreeNode));

	readFile(tree, toPrint, root);
	dispNode(toPrint);

	// Chama recursivamente para todos os filhos de root
	for (int i = 0; i < 2 * t; i++) traverse(tree, toPrint->children[i]);

	free(toPrint);
	}


// Imprime as informações de um nó da árvore BTree
void dispNode(bTreeNode *node) {

	printf("Position in node:%d\n", node->pos);
	printf("Number of Records:%d\n", node->noOfRecs);
	printf("Is leaf?:%d\n", node->isLeaf);
	printf("Keys:\n");
	for (int i = 0; i < node->noOfRecs; i++) printf("%d ", node->keyRecArr[i]);
	printf("\n");
	printf("Links:\n");
	for (int i = 0; i < 2 * t; ++i) printf("%d ", node->children[i]);
	printf("\n\n");
}


// Procura um valor-chave na árvore (bTree) a partir de um nó. Primeiro há um looping para descobrir se o valor-chave está entre os registo de valores da árvore. Em seguida, verifica-se se o valor foi encontrado. Se caso foi encontrado, retorna o registo. Caso contrário, verifica se é uma folha e retorna NULL pois não possui mais filhos para procurar.
recordNode* searchRecursive(bTree* tree, int key, bTreeNode* node) {
    
	int i = 0; // contador
    
	// Itera sobre todas as keys do nó passado até o valor procurado ser maior do que as keys do nó
	while(i < node->noOfRecs && key > node->keyRecArr[i]) i++;

	// Se não passamos por todas as chaves do nó e o valor procurado é o mesmo que keys[i], achamos nosso valor
	if(i < node->noOfRecs && key == node->keyRecArr[i]) return node->posRecArr[i];

	// Se passamos por todos os valores do nó e o nó é uma folha, então o valor não está na árvore
	else if(node->isLeaf) return NULL;

	// Se não, procuramos o valor no nó apontado pela última key do nó passado, já que key > valores[i]
	else {
        
		// Trás para a memória principal o nó da próxima busca a ser feita 
		bTreeNode* childAtPosi = malloc(sizeof(bTreeNode));
		readFile(tree, childAtPosi, node->children[i]);

		// Faz a busca recursiva
		recordNode* found = searchRecursive(tree, key, childAtPosi);
		free(childAtPosi);
		return found;
	}
}


// Se quisermos procurar na raiz da árvore, passamos a árvore e a key que queremos procurar
recordNode* search(bTree* tree, int key) {
    
	// Coloca a raíz da árvore em memória principal
	bTreeNode* root = malloc(sizeof(bTreeNode));
	readFile(tree, root, tree->root);

	// Chama a busca recursiva
	recordNode* result = searchRecursive(tree, key, root);

	free(root);

	return result;     
}


// Retorna o índice da key mais próxima à procurada em um nó. Para isso percorre todas as keys e para na que seja a antecessora (se k existir no nó) ou até o último índice de key no nó (caso k não exista no nó passado)
int findKey(bTreeNode* node, int k) {
	
	int idx = 0;
	while (idx < node->noOfRecs && k > node->keyRecArr[idx]) idx++;
	return idx;
}


// Essa função "Anda um espaço para trás" na Lista de Registros de uma folha (i.e. faz o pop() do último elemento) e por fim, atualiza o novo número de registros.
void removeFromLeaf(bTree* tree, bTreeNode* node, int idx) {
  
	for (int i = idx + 1; i < node->noOfRecs; ++i) node->posRecArr[i-1] = node->posRecArr[i];

	node->noOfRecs--;
}


// Remove uma key de um nó que não seja uma folha, passando um nó e o índice da key naquele nó
void removeFromNonLeaf(bTree *tree, bTreeNode *node, int idx) {

	int k = node->keyRecArr[idx];

	bTreeNode *child = malloc(sizeof(bTreeNode));
	bTreeNode *sibling = malloc(sizeof(bTreeNode));

	readFile(tree, child, node->children[idx]);
	readFile(tree, sibling, node->children[idx + 1]);

	if (child->noOfRecs >= t) {
		recordNode* pred = getPred(tree, node, idx);
		node->posRecArr[idx] = pred;
		removeNode(tree, child, pred->codigoLivro);
	}

	else if (sibling->noOfRecs >= t) {
		recordNode* succ = getSucc(tree, node, idx);
		node->posRecArr[idx] = succ;
		removeNode(tree, sibling, succ->codigoLivro);
	}

	else {
		child = merge(tree, node, idx);
		removeNode(tree, child, k);
		return;
	}

	writeFile(tree, child, child->pos);
	writeFile(tree, sibling, sibling->pos);

	free(child);
	free(sibling);
}


// Remove um Node de uma árvore a partir de uma key
void removeNode(bTree *tree, bTreeNode *node, int k) {

	int idx = findKey(node, k);
	
	if (idx < node->noOfRecs && node->keyRecArr[idx] == k) {
    
		if (node->isLeaf) removeFromLeaf(tree, node, idx);
		else removeFromNonLeaf(tree, node, idx);

		writeFile(tree, node, node->pos);
	}
  
	else {

		if (node->isLeaf) return;

		// Se indíce da key a ser removida for igual ao número de chaves no nó, significa que a chave procurada não pertence ao nó e flag=True
		bool flag = idx == node->noOfRecs;

		bTreeNode* childAtPosi = malloc(sizeof(bTreeNode));
		readFile(tree, childAtPosi, node->children[idx]);

		if (childAtPosi->noOfRecs < t) {
			fill(tree, node, idx);
			readFile(tree, childAtPosi, node->children[idx]);
		}

		// TEMP COMMENT: não faz sentido, não tem como "flag" ser igual e maior ao mesmo tempo que noOfRecs -> "if" nunca é executado
		if (flag && idx > node->noOfRecs) {
		
			bTreeNode* sibling = malloc(sizeof(bTreeNode));
			readFile(tree, sibling, node->children[idx - 1]);
			removeNode(tree, sibling, k);

			writeFile(tree, sibling, sibling->pos);
			free(sibling);
		}

		else removeNode(tree, childAtPosi, k);

		writeFile(tree, childAtPosi, childAtPosi->pos);
		free(childAtPosi);
	}
}



// Obtém o registro predecessor dado um índice e um nó, i.e. o registro que antecede o passado ("filho da esquerda")
recordNode* getPred(bTree *tree, bTreeNode *node, int idx) {

	// Trás o registro de index "idx" para memória principal
	bTreeNode* curr = malloc(sizeof(bTreeNode));
	readFile(tree, curr, node->children[idx]);

	// Enquanto o filho do nó dado não for uma folha, pega o "filho a esquerda"
	if (!curr->isLeaf) readFile(tree, curr, curr->children[curr->noOfRecs]);
	recordNode* result = curr->posRecArr[curr->noOfRecs - 1];

	free(curr);
	return result;
}


// Obtém o registro sucessor dado um índice e um nó, i.e. o registro seguinte do passado ("filho da direita")
recordNode* getSucc(bTree *tree, bTreeNode *node, int idx) {

	// Trás o registro de index "idx" para a memória principal
	bTreeNode *curr = malloc(sizeof(bTreeNode));
	readFile(tree, curr, node->children[idx + 1]);

	// Enquanto o filho do nó dado não for uma folha, pega o "filho da direita"
	if (!curr->isLeaf) readFile(tree, curr, curr->children[0]);
	recordNode* result = curr->posRecArr[0];

	free(curr);
	return result;
}


// função responsável por identificar os possíveis casos em que é necessário pegar emprestado um registro, seja do predecessor ou do sucessor, e realizar o preenchimento dos blocos para manter as propriedades da árvore B .
void fill(bTree *tree, bTreeNode *node, int idx) {
  
	// Pega o antecedor e sucessor de um registro
	bTreeNode *cPrev = malloc(sizeof(bTreeNode));
	bTreeNode *cSucc = malloc(sizeof(bTreeNode));

	readFile(tree, cPrev, node->children[idx - 1]);
	readFile(tree, cSucc, node->children[idx + 1]);

	if (idx != 0 && cPrev->noOfRecs >= t) borrowFromPrev(tree, node, idx);

	else if (idx != node->noOfRecs && cSucc->noOfRecs >= t) borrowFromNext(tree, node, idx);

	else {
		if (idx != node->noOfRecs) merge(tree, node, idx);
		else merge(tree, node, idx - 1);
	}

	free(cPrev);
	free(cSucc);

	return;
}


// função chamado em algum caso específico de remoção em que haja a necessidade de pegar um registro emprestado do  bloco anterior.
void borrowFromPrev(bTree *tree, bTreeNode *node, int idx) {
	
	bTreeNode *child = malloc(sizeof(bTreeNode));
	bTreeNode *sibling = malloc(sizeof(bTreeNode));

	readFile(tree, child, node->children[idx]);
	readFile(tree, sibling, node->children[idx - 1]);

	for (int i = child->noOfRecs - 1; i >= 0; --i) child->posRecArr[i + 1] = child->posRecArr[i];

	if (!child->isLeaf) for (int i = child->noOfRecs; i >= 0; --i) child->children[i + 1] = child->children[i];

	child->posRecArr[0] = node->posRecArr[idx - 1];

	if (!node->isLeaf) {
		child->children[0] = sibling->children[sibling->noOfRecs];
		sibling->children[sibling->noOfRecs] = -1;
	}

	node->posRecArr[idx - 1] = sibling->posRecArr[sibling->noOfRecs - 1];

	child->noOfRecs += 1;
	sibling->noOfRecs -= 1;

	writeFile(tree, node, node->pos);
	writeFile(tree, child, child->pos);
	writeFile(tree, sibling, sibling->pos);

	free(child);
	free(sibling);

	return;
}


// função chamado em algum caso específico de remoção em que haja a necessidade de pegar um registro emprestado do proximo bloco .
void borrowFromNext(bTree* tree, bTreeNode* node, int idx) {

	bTreeNode* child = malloc(sizeof(bTreeNode));
	bTreeNode* sibling = malloc(sizeof(bTreeNode));

	readFile(tree, child, node->children[idx]);
	readFile(tree, sibling, node->children[idx + 1]);

	child->posRecArr[(child->noOfRecs)] = node->posRecArr[idx];

	if (!(child->isLeaf)) child->children[(child->noOfRecs) + 1] = sibling->children[0];

	node->posRecArr[idx] = sibling->posRecArr[0];

	for (int i = 1; i < sibling->noOfRecs; ++i) sibling->posRecArr[i - 1] = sibling->posRecArr[i];

	if (!sibling->isLeaf) {
		
		for (int i = 1; i <= sibling->noOfRecs; ++i) sibling->children[i - 1] = sibling->children[i];

		sibling->children[sibling->noOfRecs] = -1;
	}

	child->noOfRecs += 1;
	sibling->noOfRecs -= 1;

	writeFile(tree, node, node->pos);
	writeFile(tree, child, child->pos);
	writeFile(tree, sibling, sibling->pos);

	free(child);
	free(sibling);

	return;
}


// função responsável pela união de diferentes nós com seus devidos registros, como forma de manter a estrutura propósta pela árvore B
bTreeNode *merge(bTree *tree, bTreeNode *node, int idx) {

	bTreeNode *child = malloc(sizeof(bTreeNode));
	bTreeNode *sibling = malloc(sizeof(bTreeNode));

	readFile(tree, child, node->children[idx]);
	readFile(tree, sibling, node->children[idx + 1]);

	child->posRecArr[t - 1] = node->posRecArr[idx];

	for (int i = 0; i < sibling->noOfRecs; ++i) child->posRecArr[i + t] = sibling->posRecArr[i];

	if (!child->isLeaf) for (int i = 0; i <= sibling->noOfRecs; ++i) child->children[i + t] = sibling->children[i];

	for (int i = idx + 1; i < node->noOfRecs; ++i) node->posRecArr[i - 1] = node->posRecArr[i];

	for (int i = idx + 2; i <= node->noOfRecs; ++i) node->children[i - 1] = node->children[i];
	
	node->children[node->noOfRecs] = -1;
	child->noOfRecs += sibling->noOfRecs + 1;
	node->noOfRecs--;

	if (node->noOfRecs == 0) tree->root = node->children[0];

	writeFile(tree, node, node->pos);
	writeFile(tree, child, child->pos);
	writeFile(tree, sibling, sibling->pos);

	free(sibling);

	return child;
}


// Remove uma chave de uma árvore por meio da busca
bool removeFromTree(bTree *tree, int key) {
  
	// Trás a raíz para memória principal
	bTreeNode* root = malloc(sizeof(bTreeNode));
	readFile(tree, root, tree->root);

	// Verifica se o valor está na árvore
	bool found = search(tree, key);
	if (found) removeNode(tree, root, key);

	free(root);
	return found;
}


void hardPrint(bTree *tree) {
	
	bTreeNode* lido = (bTreeNode*) malloc(sizeof(bTreeNode));
	
	for (int i = 0; i < tree->nextPos; i++) {
	
		fseek(tree->fp, i * sizeof(bTreeNode), SEEK_SET);
		fread(lido, sizeof(bTreeNode), 1, tree->fp);

		if (lido->isLeaf <= 1) dispNode(lido);
		
		else
			printf("ERRO: isLeaf = %i\n\n", lido->isLeaf);
	}

	free(lido);
}



void doublePrint(bTree *tree) {

	printf("=================");
	printf("\nTraverse\n");
	traverse(tree, tree->root);

	printf("=================");
	printf("\nHard print\n");
	hardPrint(tree);
}
