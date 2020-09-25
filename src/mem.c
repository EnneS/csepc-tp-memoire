
#include "mem.h"
#include "mem_os.h"
#include "common.h"
#include <stdio.h>
#include <string.h>

typedef struct fb{  /* fb pour free block */
    size_t size ;
    struct fb *next ;
    /* ... */
} fb;

typedef struct bb {
    size_t size;
    struct bb* next;
} bb;

typedef struct entete {
    struct fb* fb_first;
    struct bb* bb_first;
} entete;

void * memory;

//-------------------------------------------------------------
// mem_init
//-------------------------------------------------------------
void mem_init() {
    memory = get_memory_adr();
    
    entete* entete;
    entete = memory;

    fb* init_fb; 
    init_fb = memory + sizeof(entete);

    init_fb->size = get_memory_size() - sizeof(entete);

    entete->fb_first = init_fb;
    entete->bb_first = NULL;
}

//-------------------------------------------------------------
// mem_alloc
//-------------------------------------------------------------
void* mem_alloc(size_t size) {
    /* A COMPLETER */ 
    entete* head = (entete*) get_memory_adr();
    fb* current = (fb*)head->fb_first;
    fb* last = NULL;
    //On parcours les free blocks
    while(current != NULL && current->size <= (size + sizeof(bb))) {
        last = current;
        current = current->next;
    } 
    //Si le fb courant a une taille supérieure à la taille demandé
    if(current) {
        bb* memory_asked = (bb*) current;
        // while()
        if(current->size > (sizeof(fb) + size + sizeof(bb))){
        //Si la taille restante est suffisant pour un free block
            //On alloue la mémoire damndé et on créer un autre free block
            fb* new;
            new = current + sizeof(bb) + size;
            new->next = current->next;

            if(last) {
                last->next = new;
            } else {
                head->fb_first = new;
            }

            memory_asked->size = size;
        } else {         
        //Sinon 
            //On alloue toute la mémoire au pour la demande
            //next du block précédant est next de courant
            if(last)
                last->next = current->next;
            else 
                head->fb_first = current->next;
        }
        return memory_asked + sizeof(bb);
        
    } else {
    //Si on a pas de block disponible on retourne null
        return NULL;
    }
}

//-------------------------------------------------------------
// mem_free
//-------------------------------------------------------------
void mem_free(void* zone) {
   /* A COMPLETER */ 

    //On soustrait a la zone la taille de bb
    //On parcours la chaine de fb et tant que courant < zone
        //Si fb courant + courant->size == zone
            //Si 

            //courant = fb
            //last->next = courant
            //courant->next

    //Si courant == null 
        //print segmentation fault
}

//-------------------------------------------------------------
// Itérateur(parcours) sur le contenu de l'allocateur
// mem_show
//-------------------------------------------------------------
void mem_show(void (*print)(void *, size_t, int free)) {
   /* A COMPLETER */ 
}

//-------------------------------------------------------------
// mem_fit
//-------------------------------------------------------------
void mem_fit(mem_fit_function_t* mff) {
   /* A COMPLETER */ 
}

//-------------------------------------------------------------
// Stratégies d'allocation 
//-------------------------------------------------------------
struct fb* mem_first_fit(struct fb* head, size_t size) {
   /* A COMPLETER */ 
    return NULL;
}
//-------------------------------------------------------------
struct fb* mem_best_fit(struct fb* head, size_t size) {
   /* A COMPLETER */ 
    return NULL;

}
//-------------------------------------------------------------
struct fb* mem_worst_fit(struct fb* head, size_t size) {
   /* A COMPLETER */ 
    return NULL;
}
