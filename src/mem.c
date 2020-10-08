
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
        printf("Fb trouvé, current : %p , current_size : %zu, size necessaire : %zu\n",current, current->size, (size + sizeof(bb) + 1));
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
    
    entete* head = (entete*) get_memory_adr();
    bb* bb_current = head->bb_first;
    bb* bb_last = NULL;
    // Chercher le busyblock correspondant à la zone donnée
    while(bb_current != NULL && (void*)((char*) bb_current + sizeof(bb)) != zone) {
        bb_last = bb_current;
        bb_current = bb_current->next;
    }
    if(bb_current) {
        fb* fb_current = head->fb_first;
        fb* fb_last = NULL;

        // Chercher le freeblock après le plus proche
        while(fb_current != NULL && fb_current < (fb*) zone) {
            fb_last = fb_current;
            fb_current = fb_current->next;
        }

        // Si il y a assez de place pour créer un freeblock
        if((bb_current->size + sizeof(bb)) >= sizeof(fb)) {
            // Si il y a un freeblock avant
            if(fb_last != NULL) {
                // Si il y a un freeblock après
                if(fb_current != NULL) {
                    // Si il y a un busyblock collé après
                    if(bb_current->next == (bb*)((char*) bb_current + bb_current->size + sizeof(bb))) {
                        //Si il y a un busyblock collé avant 
                        if(bb_last != NULL && bb_last > (bb*) fb_last) {
                            // A
                            bb_last->next = bb_current->next;
                            fb* new_fb = (fb*) bb_current;
                            new_fb->size = bb_current->size + sizeof(bb);
                            new_fb->next = fb_last->next;
                            fb_last->next = new_fb;
                        } 
                        // Sinon (fb collé avant)
                        else {
                            // B
                            if(bb_last != NULL){
                                bb_last->next = bb_current->next;
                            } else {
                                head->bb_first = bb_current->next;
                            }
                            fb_last->size += bb_current->size + sizeof(bb);
                        }
                    //Sinon (fb collé après)
                    } else {    
                        //Si il y a un busyblock collé avant
                        if(bb_last != NULL && bb_last > (bb*) fb_last) {
                            // C
                            bb_last->next = bb_current->next;
                            fb* new_fb = (fb*) bb_current;
                            new_fb->next = fb_current->next;
                            fb_last->next = new_fb;
                            new_fb->size = bb_current->size + sizeof(bb) + fb_current->size;
                        }
                        // Sinon (fb collé avant)
                        else {
                            // D
                            if(bb_last != NULL){
                                bb_last->next = bb_current->next;
                            } else {
                                head->bb_first = bb_current->next;
                            }
                            fb_last->next = fb_current->next;
                            fb_last->size += bb_current->size + sizeof(bb) + fb_current->size;
                        }
                    }
                }
                // Sinon (busyblock après ou collé à la fin)
                else {
                    //Si il y a un busyblock collé avant
                    if(bb_last != NULL && bb_last > (bb*) fb_last) {
                        // E
                        bb_last->next = bb_current->next;
                        fb* new_fb = (fb*) bb_current;
                        new_fb->size = bb_current->size + sizeof(bb);
                        new_fb->next = fb_last->next;
                        fb_last->next = new_fb;
                    } 
                    // Sinon (fb collé avant)
                    else {
                        // F
                        if(bb_last != NULL){
                            bb_last->next = bb_current->next;
                        } else {
                            head->bb_first = bb_current->next;
                        }
                        fb_last->size += bb_current->size + sizeof(bb);
                    }
                }
            }
            // Sinon
            else {
                // Si il n'existe pas de freeblock après
                if(fb_current == NULL) {
                    // Si il existe un busyblock collé avant
                    if(bb_last != NULL && bb_last > (bb*) fb_last) {
                        // G
                        bb_last->next = bb_current->next;
                    }
                    // Sinon (collé à l'entête)
                    else {
                        // H
                        head->bb_first = bb_current->next;
                    } 
                    fb* new_fb = (fb*) bb_current;
                    new_fb->next = NULL;
                    new_fb->size = bb_current->size + sizeof(bb);
                    head->fb_first = new_fb;
                }  
                // Sinon
                else {
                    // Si il existe un busyblock collé après
                    if(bb_current->next == (bb*)((char*) bb_current + bb_current->size + sizeof(bb))) {
                        // Si il existe un busyblock collé avant
                        if(bb_last != NULL && bb_last > (bb*) fb_last) {
                            // I
                            bb_last->next = bb_current->next;
                        }
                        // Sinon collé à l'entête
                        else {
                            // J
                            head->bb_first = bb_current->next;
                        }
                        fb* new_fb = (fb*) bb_current;
                        new_fb->next = head->fb_first;
                        head->fb_first = new_fb;
                        new_fb->size = bb_current->size + sizeof(bb);
                    }
                    // Sinon (fb collé après)
                    else {
                        // Si il existe un busyblock collé avant
                        if(bb_last != NULL) {
                            // K
                            bb_last->next = bb_current->next;
                        }
                        // Sinon l'entête est collé
                        else {
                            // L
                            head->bb_first = bb_current->next;
                        }
                        fb* new_fb = (fb*) bb_current;
                        new_fb->next = head->fb_first;
                        head->fb_first = new_fb;
                        new_fb->size = bb_current->size + sizeof(bb) + fb_current->size;
                    }
                }
            }
        }
    } else  {
        printf("mem_free: Segmentation fault - Zone donnée invalide\n");
    }
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
