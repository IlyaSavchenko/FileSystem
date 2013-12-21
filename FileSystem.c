#include <stdlib.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>

typedef struct Node* nodeRef; 
typedef struct Node{
	cahr* nodeName;
	char* nodeData;
	nodeRef* nodeParent;
	nodeRef nodeChild*;
	int countOfChild;
 } node;

 nodeRef treeSystem;
 nodeRef* nowNode;
 nodeRef* nowNodeData = "";

int main(int argc, char const *argv[])
{
	/* code */
	return 0;
}

nodeRef Construction()
{
    nodeRef treeSystem = (nodeRef)malloc(sizeof(node));
    treeSystem->nodeName = ""; 
    treeSystem->nodeData = 0;
    treeSystem->nodeChild = 0;
    treeSystem->nodeParent = 0;
    treeSystem->countOfChild = 0;
    return treeSystem;
}