
/*
 * Cache efficient list.
 * A CEList object consists of a list of List_section objs, each List_section
 * can in a continuous chunk of memory store a fixed number of objects each
 * of fixed size. The CEList is most like a std::vector, it supports random
 * iterator/index access of its elements, and bi-directional iteration.
 * */
typedef struct CEList
{
    uint16_t element_sz;
    uint16_t num_elements_per_section;
    uint32_t length; // NO. of elements in all sections.
    List_section *tail;
    MemoryContext *mctx; // where to allocate memory for all parts of this list.
    List_section head;
} CEList;

// each slot has one bit, 0 if the slot is NOT used, 1 if it is used. this is
// used to support element delete efficiently.
typedef union CEList_section_slot_usage_bmp
{
    char bmp[8]; // suitable for List_section with no more than 64 element.
    char *slot_use_bmp; // suitable for List_section with more than 64 elements.
} CEList_section_slot_usage_bmp;

typedef struct List_section_bufhdr
{
    uint16_t num_elements; // number of effective elements stored.
    uint16_t alloced_num_elements; // buf space
} List_section_bufhdr;

typedef struct List_section
{
    // uint32 num_elements;// number of effective elements stored.
    // this is stored in (buf-8)


    // slot allocation bitmap of num_elements_per_section bits, slot[N]'s bit
    // is at (num_elements_per_section[N/8] & (1<<N%8))
    CEList_section_slot_usage_bmp usage_bmp;
    /*
     * num_elements_per_section slots each element_sz bytes. before it there
     * is a List_section_bufhdr header, storing num_elements. many uses of List only ever
     * appends a few( < 10) elements to a list, so we can alloc only N
     * (N < CEList::num_elements_per_section) elements, and when that's consumed,
     * either repalloc more, up to num_elements_per_section elements, or,
     * insert another List_section, to avoid reallocation&memcpy, and in this case
     * we can parameterize the section buffer size instead of the NO. of elements to store,
     * so that a section may store as few as one element for huge and rare
     * elements and alloc such elements one by one on demand.
     */
    char *buf;
    struct List_section *next, *prev;
} List_section;

// A List_section's 'num_elements' field, both a left value and a right value.
#define NUM_ELEMENTS(psect) (((List_section_bufhdr*)(psect->buf - sizeof(List_section_bufhdr)))->num_elements)
#define ALLOCED_NUM_ELEMENTS(psect) (((List_section_bufhdr*)(psect->buf - sizeof(List_section_bufhdr)))->alloced_num_elements)

typedef struct List_iterator
{
    CEList *owner;
    List_section *section;
    /* index is offset within 'section', not global offset. */
    int32_t index;
    uint32_t global_index;
    // points to current element stored in section->buf
    void *element;
} List_iterator;

static void *get_element(List_section *sec, uint32_t idx);
/* n is offset within s, not global offset. */
static void set_usage_bit(List_section *s, int n, int bit);
static void append_section(CEList *l);
// two ajacent sections are merged if deletions cause both under half full.
static void merge_sections(List_section *s1, List_section *s2);
// a section is deleted if it's elements are all deleted.
static void delete_section(List_section *s1);


void CEList_begin_iteration(List_iterator *li);

/*
 * move li to next element, return true if there is next element, false if no
 * more elements.
 * */
bool CEList_move_next(List_iterator *li);
bool CEList_move_prev(List_iterator *li);
void CEList_delete_current(List_iterator *li);
void *CEList_current_element(List_iterator *li);
Oid CEList_current_element_oid(List_iterator *li);
int CEList_current_element_int(List_iterator *li);

/*
 * Make a list. if l is 0, alloc from mctx the CEList object. all internal
 * memory for l allocated from mctx. elements are of equal size, each has elesz bytes.
 * Note that internally elesz must be MAXALIGN'ed.
 * each list section has nele_section elements, recommended value is 64 or
 * less unless the list will store thousands of elements or more.
 * estimated_total_elements: this helps preallocates all List_section objects at once, 0 if unknown.
 * */
CEList *Make_celist(CEList *l, MemoryContext *mctx, uint16_t elesz,
    uint16_t nele_section, uint16_t nele_section_initial_alloc,
    uint32_t estimated_total_elements);
void *CEList_alloc_element(CEList *l);
void CEList_append_element(CEList *l, void *ele);
void CEList_delete_element(CEList *l, int n);
void *CEList_element_at(CEList *l, size_t n);
Oid CEList_oid_element_at(CEList *l, size_t n);
int CEList_int_element_at(CEList *l, size_t n);
/*
 * Free all sections within l, and l itself if it was allocated from l->mctx.
 * */
void Free_celist(CEList *l);
