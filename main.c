#include "errortable.h"

#include "linux_errno.h"
#include "windows_errno.h"
#include "winerror.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define strdup _strdup
struct mapentry {
    struct mapentry* next;
    unsigned int hash;
    char* key;
    struct error_table* p;
};

struct hashmap {
    struct mapentry** table;
    unsigned int capacity;
    int  size;
};

void hashmap_destroy(struct hashmap* pMap);
int hashmap_set(struct hashmap* pMap, const char* key, struct error_table* pNew);
struct error_table* hashmap_find(struct hashmap* pMap, const char* key);

int main()
{
    struct hashmap map = { .capacity = 5000 };

    //mapeia todos os erros posix 
    for (int i = 0; i < ARRAYLEN(linux_errno); i++)
    {
        struct error_table* p = hashmap_find(&map, linux_errno[i].name);
        if (p == 0)
        {
            hashmap_set(&map, linux_errno[i].name, &linux_errno[i]);
        }
        else
        {
            //?
        }
    }

    
    for (int i = 0; i < ARRAYLEN(windows_errno); i++)
    {
        struct error_table* p = hashmap_find(&map, windows_errno[i].name);
        if (p == 0)
        {
            hashmap_set(&map, windows_errno[i].name, &windows_errno[i]);
        }
        else
        {
            printf("'%s' already exists ", p->name);
            if (p->code != windows_errno[i].code)
            {
                printf("diferent values linux=%d windows=%d", p->code, windows_errno[i].code);
            }
            else
            {
                printf("same value %d", p->code);
            }
            printf("\n");
        }        
    }

    for (int i = 0; i < ARRAYLEN(winerror); i++)
    {
        struct error_table* p = hashmap_find(&map, winerror[i].name);
        if (p == 0)
        {
            hashmap_set(&map, winerror[i].name, &winerror[i]);
        }
        else
        {
            printf("'%s' already exists ", p->name);
            if (p->code != winerror[i].code)
            {
                printf("diferent values %d %d", p->code, winerror[i].code);
            }
            else
            {
                printf("same value %d", p->code);
            }
            printf("\n");
        }
    }

    FILE* output = fopen("unicode.h", "w");
    if (output == 0)
        return;

    struct hashmap messagemap = { .capacity = 5000 };
    struct hashmap codemap = { .capacity = 5000 };
    
    fprintf(output, "#include \"errortable.h\"\n"
                    "struct error_table linux_errno[] = { \n");


    for (int i = 0; i < map.capacity; i++)
    {
        if (map.table[i] != NULL)
        {
            struct mapentry* entry = map.table[i];
            while (entry)
            {
                char codestr[20] = {0};
                _itoa(entry->p->code, codestr, 10);

                struct error_table* codeentry = hashmap_find(&codemap, codestr);
                if (codeentry == 0)
                {
                    hashmap_set(&messagemap, codestr, entry->p);
                }
                else
                {
                    //foram adicionados dois erros com mesmo codigo
                    printf("codigo ja existe entre %s %s\n", entry->p->name, codeentry->name);
                }


                struct error_table* msgentry = hashmap_find(&messagemap, entry->p->message);
                if (msgentry == 0)
                {
                    hashmap_set(&messagemap, entry->p->message, entry->p);
                }
                else
                {
                    //foram adicionados dois erros com a mesma mensagem
                    printf("warning: mensagem igual entre %s %s\n", entry->p->name, msgentry->name);
                }

                fprintf(output, "{\"%s\", %d, \"%s\"},\n", entry->p->name, entry->p->code, entry->p->message);
                entry = entry->next;
            }
        }
    }

    fprintf(output, "};\n");
    
    fclose(output);

    hashmap_destroy(&map);
    hashmap_destroy(&messagemap);
}


unsigned int stringhash(const char* key)
{
    // hash key to unsigned int value by pseudorandomizing transform
    // (algorithm copied from STL char hash in xfunctional)
    unsigned int uHashVal = 2166136261U;
    unsigned int uFirst = 0;
    unsigned int uLast = (unsigned int)strlen(key);
    unsigned int uStride = 1 + uLast / 10;

    for (; uFirst < uLast; uFirst += uStride)
    {
        uHashVal = 16777619U * uHashVal ^ (unsigned int)key[uFirst];
    }

    return (uHashVal);
}


void hashmap_remove_all(struct hashmap* pMap)
{
    if (pMap->table != NULL)
    {
        for (unsigned int i = 0; i < pMap->capacity; i++)
        {
            struct mapentry* pentry = pMap->table[i];

            while (pentry != NULL)
            {
                struct mapentry* pentryCurrent = pentry;

                

                pentry = pentry->next;
                free(pentryCurrent->key);
                free(pentryCurrent);
            }
        }

        free(pMap->table);
        pMap->table = NULL;
        pMap->size = 0;
    }
}

void hashmap_destroy(struct hashmap* pMap)
{
    hashmap_remove_all(pMap);
}

struct error_table* hashmap_find(struct hashmap* pMap, const char* key)
{
    if (pMap->size == 0)
        return NULL;

    struct error_table* p = NULL;

    unsigned int hash = stringhash(key);
    int index = hash % pMap->capacity;

    struct mapentry* pentry = pMap->table[index];

    for (; pentry != NULL; pentry = pentry->next)
    {
        if (pentry->hash == hash && strcmp(pentry->key, key) == 0) {
            p = pentry->p;
            break;
        }
    }

    return p;
}


struct error_table* hashmap_remove(struct hashmap* map, const char* key)
{
    struct error_table* p = 0;

    if (map->table != NULL)
    {
        unsigned int hash = stringhash(key);
        struct mapentry** preventry = &map->table[hash % map->capacity];
        struct mapentry* pentry = *preventry;

        for (; pentry != NULL; pentry = pentry->next)
        {
            if ((pentry->hash == hash) && (strcmp(pentry->key, key) == 0))
            {
                *preventry = pentry->next;
                p = pentry->p;
                free(pentry->key);
                free(pentry);
                break;
            }
            preventry = &pentry->next;
        }
    }

    return p;
}

int hashmap_set(struct hashmap* pMap, const char* key, struct error_table* pNew)
{
    int result = 0;

    if (pMap->table == NULL)
    {
        if (pMap->capacity < 1) {
            pMap->capacity = 1000;
        }

        pMap->table = calloc(pMap->capacity, sizeof(pMap->table[0]));
    }

    if (pMap->table != NULL)
    {
        unsigned int hash = stringhash(key);
        int index = hash % pMap->capacity;

        struct mapentry* pentry = pMap->table[index];

        for (; pentry != NULL; pentry = pentry->next) {
            if (pentry->hash == hash && strcmp(pentry->key, key) == 0) {
                break;
            }
        }

        if (pentry == NULL)
        {
            pentry = calloc(1, sizeof(*pentry));
            pentry->hash = hash;
            pentry->p = pNew;
            pentry->key = strdup(key);
            pentry->next = pMap->table[index];
            pMap->table[index] = pentry;
            pMap->size++;
            result = 0;
        }
        else
        {
            result = 1;
            pentry->p = pNew;
        }
    }

    return result;
}
