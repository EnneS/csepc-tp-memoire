
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
    init_fb = (fb*)((char*) memory + sizeof(struct entete));
    init_fb->size = get_memory_size() - sizeof(entete);

    entete->bb_first = NULL;
    entete->fb_first = init_fb;
}

//-------------------------------------------------------------
// mem_alloc
//-------------------------------------------------------------
void* mem_alloc(size_t size) {
    entete* head = (entete*) get_memory_adr();
    fb* current = (fb*)head->fb_first;
    fb* last = NULL;

    //On parcours les free blocks
    while(current != NULL && current->size < (size + sizeof(bb))) {
        last = current;
        printf("Fb trouvé, current_size : %zu, size necessaire : %zu\n",current->size, (size + sizeof(bb) + 1));
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
            new = (fb*) ((char*) current + sizeof(bb) + size);
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
        return (char*) memory_asked + sizeof(bb);
        
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
    while(bb_current != NULL && (void*)((char*) bb_current + sizeof(bb)) != zone) {
        bb_last = bb_current;
        bb_current = bb_current->next;
    }
    if(bb_current) {
        fb* fb_current = head->fb_first;
        fb* fb_last = NULL;

        while(fb_current != NULL && fb_current < (fb*) zone) {
            fb_last = fb_current;
            fb_current = fb_current->next;
        }



        //SI LA ZONE A LIBERE EST SUFFISANT POUR UN ENTETE DE FREE BLOCK
        if((bb_current->size + sizeof(bb)) >= sizeof(fb)) {
            // Si bb_last == NULL et fb_last == NULL
            // (Le bloc à libérer est collé à l'entête)
            if(bb_last == NULL && fb_last == NULL) {
                // Si le bloc d'après est occupé
                if((bb*)((char*) bb_current + sizeof(bb) + bb_current->size) == bb_current->next){
                    fb* new_fb = (fb*) bb_current;
                    new_fb->size = bb_current->size + sizeof(bb);
                    new_fb->next = head->fb_first;
                    head->fb_first = new_fb;
                } else {
                    // Sinon, il s'agit d'un bloc libre
                    fb_current = (fb*) bb_current;
                    fb_current->size += bb_current->size + sizeof(bb);
                }
            } else {
                /* 1    Avant : FB
                *       Après : FB */ 
               if(fb_last > (fb*) bb_last && fb_current->next == (fb*)((char*) bb_current + sizeof(bb) + bb_current->size)){
                    fb_last->size += bb_current->size + sizeof(bb) + fb_current->size;
                    fb_last->next = fb_current->next;
                    if(bb_last == NULL){
                        head->bb_first = bb_current->next;
                    } else {
                        bb_last->next = bb_current->next;
                    }
               } 
                /* 2    Avant : FB
                *       Après : BB ou Rien */   
               else if (bb_current->next == NULL || bb_current->next == (bb*)((char*) bb_current + sizeof(bb) + bb_current->size)) {
                    fb_last->size += bb_current->size + sizeof(bb);
                    if(bb_last == NULL){
                        head->bb_first = bb_current->next;
                    } else {
                        bb_last->next = bb_current->next;
                    }
               }
               
                /* 3    Avant : BB
                *       Après : FB */
                if(bb_last > (bb*) fb_last){
                    if(fb_current->next == (fb*)((char*) bb_current + sizeof(bb) + bb_current->size)){
                        
                    } else {

                    }
                }
                /* 4    Avant : BB
                *       Après : BB ou Rien */
                else if (bb_current->next == NULL || bb_current->next == (bb*)((char*) bb_current + sizeof(bb) + bb_current->size)) {

                }
            }


            //1 SI LE BLOCK D'AVANT EST UN FREE BLOCK ET LE BLOCK D'APRÈS EST UN FREE BLOCK
            if(fb_last->next == (fb*)((char *) bb_current +  bb_current->size + sizeof(bb)) && fb_last > (fb*) bb_last) {
                fb_last->size += bb_current->size + sizeof(bb) + fb_current->size;
                fb_last->next = fb_current->next;
                //SI LE BUSY D'AVANT EST NULL 
                if(bb_last == NULL) {
                    head->bb_first = bb_current->next;
                } 
                //SINON
                else {
                    bb_last->next = bb_current->next;
                }
            }
            //2 SI LE BLOCK D'AVANT EST UN FREE BLOCK ET LE BLOCK D'APRÈS EST UN BUSY BLOCK OU EST NULL
            else if(fb_last > (fb*) bb_last && (
            bb_last->next == (bb*)((char *) bb_current +  bb_current->size + sizeof(bb)) 
            || bb_current->next == NULL 
            )) {
                fb_last->size += bb_current->size + sizeof(bb);
                bb_last->next = bb_current->next;
            }
            //3 SI LE BLOCK D'AVANT EST UN BUSY BLOCK 
            //ET LE BLOCK D'APRÈS EST UN BUSY BLOCK OU NULL
            else if(fb_last < (fb*) bb_last && (
            bb_current->next == (bb*)((char *) bb_current +  bb_current->size + sizeof(bb)) 
            || bb_current->next == NULL
            )){
                fb* new_fb = zone;
                new_fb->next = fb_last->next;
                new_fb->size = bb_current->size + sizeof(bb);
                fb_last->next = new_fb;
            }
            //4 SI LE BLOCK D'AVANT EST UN BUSY BLOCK ET LE BLOCK D'APRÈS EST UN FREE BLOCK
            else if(bb_last > (bb*)fb_last && bb_current->next == (bb*)((char*) bb_current + sizeof(bb) + bb_current->size)) {
                fb_current = (fb*) bb_current;
                fb_current->size += bb_current->size + sizeof(bb);
            }
            //5 SI LE BLOCK D'AVANT EST L'ENTÊTE ET LE BLOCK D'APRÈS UN FREE BLOCK
            else if(bb_last == NULL && fb_last == NULL && (fb*)((char*) bb_current + sizeof(bb) + bb_current->size) == fb_current->next) {
                fb_current = (fb*) bb_current;
                fb_current->size += bb_current->size + sizeof(bb);
                head->bb_first = bb_current->next;
            }
            //5 SI LE BLOCK D'AVANT EST L'ENTÊTE ET LE BLOCK D'APRÈS UN BUSY BLOCK
            else if(bb_last == NULL && fb_last == NULL && (bb*)((char*) bb_current + sizeof(bb) + bb_current->size) == bb_current->next) {
                fb* new_fb = (fb*) bb_current;
                new_fb->next = head->fb_first;
                new_fb->size = bb_current->size + sizeof(bb);
                head->bb_first = bb_current->next;
                head->fb_first = new_fb;
            }
            //6 SI LE FREE BLOCK D'AVANT EST NULL ET LE BLOCK D'AVANT ET UN BUSY BLOCK 
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
