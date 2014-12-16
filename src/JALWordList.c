#include <pebble.h>
#include "JALWordList.h"

typedef struct WLListNode {
	char *word;
	struct WLListNode *next;
	struct WLListNode *prev;
} WLListNode;

// ivars
static WLListNode *head;
static WLListNode *tail;
static WLListNode *curr_node;
static size_t list_size;

// private functions
	// Allocates and initializes a new ListNode.
WLListNode * _wl_ListNode_ctor();
	// Deallocates a ListNode and its word.
void _wl_ListNode_dtor(WLListNode *node);
	// Deletes an entire chain of ListNodes.
void _wl_destroy_chain(WLListNode *curr);


void wl_init()
{
	// create a single NULL-filled ListNode to use as the head.
	head = tail = curr_node = _wl_ListNode_ctor();
	list_size = 1;
}


void wl_deinit()
{
	_wl_destroy_chain(head);
	head = tail = curr_node = NULL;
	list_size = 0;
}


const char * wl_add_word(const char *newWord)
{
	if (!newWord) {
		return NULL;
	}
	size_t new_word_length;
	WLListNode *new_node = _wl_ListNode_ctor();
	if (!new_node) { // safety first
		goto LogFailure;
	}
	new_word_length = strlen(newWord);
	// make our own copy of the word
	new_node->word = malloc(sizeof(char) * new_word_length);
	if (!new_node->word) { // safety first
		// not enough memory to copy the new word
		free(new_node);
		new_node = NULL;
		goto LogFailure;
	}
	// copy the old word into the new buffer
	strncpy(new_node->word, newWord, new_word_length);
	// add the new node to the back of the linked list
	tail->next = new_node;
	new_node->prev = tail;
	// update the tail pointer
	tail = new_node;
	return new_node->word;
	
LogFailure:
	APP_LOG(APP_LOG_LEVEL_DEBUG, "%s Unable to add new word.", __func__);
	return NULL;
}


const char * wl_next_word()
{
	if (curr_node == tail) {
		return NULL;
	}
	curr_node = curr_node->next;
	return curr_node->word;
}


const char * wl_prev_word()
{
	if (curr_node == head) {
		return NULL;
	}
	curr_node = curr_node->prev;
	return curr_node->word;
}


WLListNode * _wl_ListNode_ctor()
{
	WLListNode *new_node = calloc(1, sizeof(WLListNode));
	if (!new_node) {
		return NULL;
	}
	new_node->word = NULL;
	new_node->next = NULL;
	new_node->prev = NULL;
	return new_node;
}


void _wl_ListNode_dtor(WLListNode *node)
{
	// safety check
	if (!node) {
		return;
	}
	// remove this node from the chain
	if (node->prev) {
		node->prev->next = node->next;
	}
	if (node->next) {
		node->next->prev = node->prev;
	}
	// dealloc the word and this node
	free(node->word);
	free(node);
}


void _wl_destroy_chain(WLListNode *curr)
{
	if (!curr) {
		return;
	}
	// recurse
	_wl_destroy_chain(curr->next);
	// delete this node
	free(curr->word);
	free(curr);
}
