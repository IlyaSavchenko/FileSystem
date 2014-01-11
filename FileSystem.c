#include <stdlib.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "fileSystem.h"

char* fs = "./device";

FILE* fileSystem() {
    FILE* sys = fopen(fs, "rb+");
    fseek(fs, 0, SEEK_SET);
    return fs;
}

char** split(char* path)
{
    char** array; // возвращаемое значение. 
 
    if (strlen(path) > 1) // возм. - "если это не корневая папка" - если длинна пути больше единицы
    {   
        int count = 0;  
        int i = 0;  
        while(path[i] != 0)
        {
            if (path[i] == '/') count++; // определяем "глубину" переданного  пути
            i++;
        }
 /* в результате мы посчитали "глубину залегания" последнего элемента - узла, указанного в пути */
 
        array = (char**)malloc(sizeof(char*)*(count + 2)); //  выделяем память для  (count + 2) указателей на массивы символов
        array[count + 1] = 0; // в конец строки - отделяющию null символ
 
        array[0] = "/"; // в начало добавлям разделяющий слэш
        int n = 1; // индекс элемента массива указателей на массивы символов - сейчас поставлен на указатель для первого массива символов.
        i = 1; // текущий индекс символа (парвый=0) в строке, содежащей переданный в функцию путь
 
 
        while(path[i] != 0)// перебираем все символы строки , содержащей путь со второго до предпоследнего включительно
        {
            int c = 1;  // число символом в имени одного из узлов пути (можно сказать - имени папки) - пока только один 
            int j = i;  // вспомогательный  индекс - далее мы считываем имя узла  - пока не встретим слыш или строка не закончится - приравниваем его к i
         
            while((path[j] != '/') && (path[j] != 0)) // определяем длинну имени узла -перемещаемся по фрагмента строки пути до следующего слэша
            {
                c++; // фиксируем то , что нами обнаружен очередной символ имени узла
        j++; // накручиваем счётчик 
        i++; // фиксируем тот факт, что мы считали очередной символ переданной в функцию строки, содержащей путь.
            }
            if (path[i] != 0) i++; /* если мы вышли из предыдущего цикла только по причине того, что встретили слэш - то пропускаем его - 
                              чтобы при следующей итерации цикла while(path[i] != 0) снова начать читать символы */
 
            array[n] = (char*)malloc(sizeof(char)*c);// выделяем память по числу букв "c" в имени текущего узла 
            array[n][c - 1] = 0;  // в конец фрагмента - имени узла - отделяющию null символ
            n++;    
        }
 
 
        n = 1; // номер указателя в массие фрагментов пути -  имён директорий
        i = 1;
        while(path[i] != 0)
        {           
            int j = i;
            int tmp = i;            
            while((path[j] != '/') && (path[j] != 0)) // заполняем массив именами фрагментов пути - именами узлов (директорий)
            {
                array[n][j - tmp] = path[j];
                j++;i++;
            }
            if (path[i] != 0) i++;
            n++;    
        }
    }
 else // если всё-таки длина пути равна единице - то есть это путь к корню относительно корня.
    {
        array = (char**)malloc(sizeof(char*)*2); // выделяем память для массива без значащего содержимого
        array[1] = 0;
        array[0] = "/";
    }
    return array;
}
 
/* ищет в иерархихи узел , путь к которому задан как (* path) -
возвращает ПОЛСЕДНИЙ ОБНАРУЖЕННЫЙ узел пути - самую "младший" во ввложенности обнаруженный файл*/

inode readInode(unsigned long index) {
    if (index == NULL) {
        log_msg("WARNING! index = NULL");
        return NULL;
    }
    FILE * fs = fileSystem();
    fseek(fs, index, SEEK_SET);
    inode in = malloc(sizeof(struct inode_s));
    fread(in, sizeof(struct inode_s), 1, fs);
    if (in->type == 0) {
        log_msg("WARNING! type of inode -> NULL");
        return NULL;
    }
    fclose(fs);
    return in;
}

void copyName(char* dest, char* source) {
    printf("copy name\n");
    int len = strlen(source);
    printf("source %s\n", source);
    len = len > 31 ? 31 : len;
    int i = 0;
    for (; i < len; i++)
        dest[i] = source[i];
    dest[i] = NULL;
    printf("dest %s\n", dest);
}

node readTree(unsigned long index, node parent, char* name) {
    if (index == NULL) return NULL;
    inode in = readInode(index);
    if (in == NULL) {
        log_msg("!!readed node is NULL");
        return NULL;    
    } 
    inode p = in;
    node head = malloc(sizeof(struct node_s));
    node h = head;
    h->parent = parent;
    h->index = index;
    h->inode = p;
    copyName(h->name, name);
    h->next = readTree(p->next, parent, name);
    if (p->type == 1) {
        for (int i = 0; i < 10; i++) {
            if (p->is_folder.nodes[i] != NULL) {
                h->childs[i] = readTree(p->is_folder.nodes[i], head, p->is_folder.names[i]);
            } else {
                h->childs[i] = NULL;
            }
        }
    }
    return h;
}

void loadFileSystem() {
    FILE * sys = fileSystem();
    fread(&fs_info, sizeof(struct fs_info_s), 1, sys);
    printf("size %d, start %d\n", fs_info.inode_size, fs_info.inode_start);
    printf("data_start %lu, dev_size %lu\n", fs_info.data_start, fs_info.dev_size);
    log_msg("");
    fs_cash = readTree(fs_info.inode_start, NULL, "/");
    fclose(sys);
}

node readNode(unsigned long index) {
    
}

void showNode(node n) {
    printf("Node\n");
    printf("~index: %lu\n", n->index);
    printf("~name: %s\n", n->name);
    if (n->inode->type == 1) {
        printf("~This is directory\n");
        for (int i = 0; i < 10; i++) {
            printf("~child [%d] is NULL %d\n", i, n->childs[i] == NULL);
            printf("~child of inode [%d] is NULL %d\n", i, n->inode->is_folder.nodes[i] == NULL);
            if (n->inode->is_folder.nodes[i] != NULL) 
                printf("~child's name [%d] %s\n", i, n->inode->is_folder.names[i]);
        }
    }
}