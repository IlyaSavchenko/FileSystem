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

nodeRef ConstructionTree()
{
    nodeRef treeSystem = (nodeRef)malloc(sizeof(node));
    treeSystem->nodeName = ""; 
    treeSystem->nodeData = 0;
    treeSystem->nodeChild = 0;
    treeSystem->nodeParent = 0;
    treeSystem->countOfChild = 0;
    return treeSystem;
}

void AdditionNode(nodeRef Parent, nodeRef Node)
{   
    Node->nodeParent = Parent;   
    nodeRef* addChild = (nodeRef*)malloc(sizeof(nodeRef*)*(parent->countOfChild + 1));
    for(i = 0; i < Parent->countOfChild; i++)
    {
        addChild[i] = Parent->nodeChild[i];
    }
    addChild[Parent->countOfChild] = Node; 
    Parent->nodeChild = addChild;
    Parent->countOfChild++; 
}

nodeRef ConstructionNode(char* name, char* data)
{
    nodeRef Node = (nodeRef)malloc(sizeof(node));
    node->nodeName = name;
    node->nodeData = content;
    node->nodeParent = 0;
    node->nodeChild = 0;
    node->countOfChild = 0;
    return node;
}

void deleteNode(nodeRef Node)
{
    nodeRef Parent = Node->nodeParent;
    nodeRef* addNode = (nodeRef*)malloc(sizeof(nodeRef)*(Parent->countOfChild - 1));  
    int mod = 0;
    for(int i = 0; i < Parent->countOfChild; i++)
    {
        if (strcmp(Parent->nodeChild[i]->nodeName, Node->nodeName) == 0)
        {
            mod = -1;
            continue;
        }
        addNode[i + mod] = Parent->nodeChild[i];
    }
    Parent->countOfChild--;
    Parent->nodeChild = addNode;
    return 0;
}

char** separationPath(char* route)
{
	char** res;
	if (strlen(path) > 1) {
		int count = 0;
		int i = 0;
		while(path[i] != 0)
			if (path[i++] == '/') 
				count++;
		res = malloc(sizeof(char*)*(count + 2));
		res[count + 1] = 0;
		res[0] = "/";
		char* pointer = strtok(path, "/");
		i = 1;
		while(pointer) {
			res[i++] = pointer;
			pointer = strtok(NULL, "/");
		}
	} else {
		res = (char**)malloc(sizeof(char*)*2);
		res[1] = 0;
		res[0] = "/";
	} 
	return res;
}

nodeRef skNode(char* path)
{
	char** res;
	if (strlen(path) > 1) {
		int count = 0;
		int i = 0;
		while(path[i] != 0)
			if (path[i++] == '/') 
				count++;
		res = malloc(sizeof(char*)*(count + 2));
		res[count + 1] = 0;
		res[0] = "/";
		char* pointer = strtok(path, "/");
		i = 1;
		while(pointer) {
			res[i++] = pointer;
			pointer = strtok(NULL, "/");
		}
	} else {
		res = (char**)malloc(sizeof(char*)*2);
		res[1] = 0;
		res[0] = "/";
	} 
	return res;
}

nodeRef seekNode(nodeRef tree, char* path) 
{   
    char** sp = separationPath(path); 
    int count = 0;
    int i = 0;
    while(sp[i++] != 0){
    	count++;
    }
    nodeRef node = skNode(tree, path);  
    if (strcmp(node->nodeName, sp[count - 1]) != 0){
    	return 0; 
    }
    return node;
}

char* memcpu(char* source, char* buf)
{
    int size = strlen(source) + strlen(buf) + 1;    
    char* result = (char*)malloc(sizeof(char)*size);
    result[size - 1] = 0; 
    int i = 0;
    for(; i < strlen(source); i++) 
    {
        result[i] = source[i];
    }
    for(; i < size; i++) 
    {
        result[i] = buf[i - strlen(source)];
    }
    return result; 
}

static struct fuse_operations fs_command =  
{
    .getattr    = fs_getattr,
};

static int fs_getattr(const char *path, struct stat *attr)
{
    memset(attr, 0, sizeof(struct stat));
    Link node = seekNode(tree, path);
    if (node == 0) return -ENOENT;  
    if (node->content == 0)
    {
        attr->st_mode = S_IFDIR | 0666;
        attr->st_nlink = 2;
    } else
    {
        attr->st_mode = S_IFREG | 0666;
        attr->st_nlink = 1;
        attr->st_size = strlen(node->content);
    }
    return 0; 
}

