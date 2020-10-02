
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
    init_fb = memory + sizeof(struct entete);
    init_fb->size = get_memory_size() - sizeof(entete);

    entete->bb_first = NULL;
    entete->fb_first = init_fb;
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
        bb* bb_last = NULL;
        bb* bb_current = head->bb_first;
        while(bb_current != NULL && bb_current < memory_asked) {
            bb_last = bb_current;
            bb_current = bb_current->next;
        }

        memory_asked->next = bb_current;
        if(bb_last == NULL) {
            head->bb_first = memory_asked;
        } else {
            bb_last->next = memory_asked;
        }

        if(current->size > (sizeof(fb) + size + sizeof(bb))){
        //Si la taille restante est suffisant pour un free block
            //On alloue la mémoire demandée et on créer un autre free block
            fb* new;
            new = current + sizeof(bb) + size;
            printf("current: %p, new : %p, size: %ld\n", current, new,current->size);
            new->size = current->size - sizeof(bb) - size;
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
            //next du block précédent est next de courant
            if(last)
                last->next = current->next;
            else 
                head->fb_first = current->next;
            
            memory_asked->size = current->size;
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
    
    entete* head = (entete*) get_memory_adr();
    bb* bb_current = head->bb_first;
    bb* bb_last = NULL;

    while(bb_current != NULL && bb_current != zone) {
        bb_last = bb_current;
        bb_current = bb_current->next;
    }
    if(bb_current) {

        if(bb_last != NULL) {
            bb_last->next = bb_current->next;
        } else {
            head->bb_first = bb_current->next;
        }
        fb* fb_current = head->fb_first;
        fb* fb_last = NULL;

        while(fb_current != NULL && fb_current < (fb*) zone) {
            fb_last = fb_current;
            fb_current = fb_current->next;
        }

        //SI LA ZONE A LIBERE EST SUFFISANT POUR UN ENTETE DE FREE BLOCK
        if(bb_current->size > sizeof(fb)) {
            //ON TROUVE LE fb D'AVANT
            //ON INSERT LE fb
            fb* new_fb = zone;
            new_fb->size = bb_current->size + sizeof(bb);
            if(fb_last != NULL) {
                new_fb->next = fb_last->next;
                fb_last->next = new_fb;
            } else {
                new_fb->next = head->fb_first;
                head->fb_first = new_fb;
            }
        } else {
        //SINON
            if(head + sizeof(entete) != zone) {
                //LE BLOCK D'AVANT EST UN bb
                    //ON AJOUTE A LA TAILLE DE CE bb LA TAILLE LIBERE ET ON CHANGE NEXT AVEC NEXT DE CURRENT
                //LE BLOCK D'AVANT EST UN FB
                    //ON AJOUTE A LA TAILLE DE CE fb LA TAILLE LIBERE ET ON CHANGE NEXT AVEC NEXT DE CURRENT
                if((void*) bb_last < (void*) fb_last) {
                    fb_last->size += bb_current->size + sizeof(bb);
                } else {
                    bb_last->size += bb_current->size + sizeof(bb);
                }
            }  
        }
    } else  {
        printf("mem_free: Segmentation fault\n");
    }

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
