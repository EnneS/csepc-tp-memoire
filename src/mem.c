
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

    // On cherche le premier freeblock ayant une taille suffisante
    fb* current = (fb*)head->fb_first;
    fb* last = NULL;
    while(current != NULL && current->size < (size + sizeof(bb))) {
        last = current;
        current = current->next;
    } 

    //Si le fb courant a une taille supérieure à la taille demandée
    if(current) {
        // On créé un busyblock pour la nouvelle zone mémoire
        bb* memory_asked = (bb*) current;
        
        // On récupère les busyblocks voisins à cette nouvelle zone mémoire
        bb* bb_last = NULL;
        bb* bb_current = head->bb_first;
        while(bb_current != NULL && bb_current < memory_asked) {
            bb_last = bb_current;
            bb_current = bb_current->next;
        }

        //Si la taille restante est suffisante pour un freeblock (non vide)
        if(current->size > (sizeof(fb) + size + sizeof(bb))){
            // On créé un nouveau freeblock
            fb* new;
            new = (fb*) ((char*) current + sizeof(bb) + size);
            new->size = current->size - sizeof(bb) - size;

            // Insertion du freeblock dans la liste chaînée
            new->next = current->next;
            if(last) {
                last->next = new;
            } else {
                head->fb_first = new;
            }

            // On alloue la mémoire
            memory_asked->size = size;
        } 
        //Sinon 
        else {     
            // Suppression du freeblock courant de la liste chaînée
            if(last)
                last->next = current->next;
            else 
                head->fb_first = current->next;

            // On alloue toute la mémoire restante du freeblock courant
            memory_asked->size = (current->size - sizeof(bb));
        }

        // On insère le nouveau busyblock dans la liste chaînée
        memory_asked->next = bb_current;
        if(bb_last == NULL) {
            head->bb_first = memory_asked;
        } else {
            bb_last->next = memory_asked;
        }

        // On renvoie le pointeur à cette zone mémoire (en n'oubliant pas d'omettre l'entête)
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

    // Chercher le busyblock correspondant à la zone donnée
    bb* bb_current = head->bb_first;
    bb* bb_last = NULL;
    while(bb_current != NULL && (void*)((char*) bb_current + sizeof(bb)) != zone) {
        bb_last = bb_current;
        bb_current = bb_current->next;
    }

    // Si on a trouvé le busyblock correspondant
    if(bb_current) {
        // Récupérere les freeblocks voisins à ce busyblock
        fb* fb_current = head->fb_first;
        fb* fb_last = NULL;
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
                        new_fb->size = bb_current->size + sizeof(bb);
                        head->fb_first = new_fb;
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
                        new_fb->next = NULL;
                        new_fb->size = bb_current->size + sizeof(bb) + fb_current->size;
                        head->fb_first = new_fb;
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
