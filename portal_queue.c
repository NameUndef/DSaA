#include "portal_queue.h"

static void PQ_pre_init(PortalQueue* dest, size_t segment_capacity, bool free_empty_segments, bool reset_popped_values)
{
    dest->free_segments = NULL;
    dest->used_segments_ring = NULL;
    dest->segment_capacity = segment_capacity;
    dest->head = 0;
    dest->tail = 0;
    dest->window_size = segment_capacity;
    dest->cur_segment_head = NULL;
    dest->cur_segment_tail = NULL;
    dest->segment_portal_src = NULL;
    dest->segment_portal_dest = NULL;
    dest->full = false;
    dest->free_empty_segments = free_empty_segments;
    dest->reset_popped_values = reset_popped_values;
}

static void PQ_post_init(PortalQueue* dest)
{
    dest->cur_segment_head = dest->used_segments_ring;
    dest->cur_segment_tail = dest->used_segments_ring;
}

static void PQ_post_deinit(PortalQueue* dest) 
{
    dest->free_segments = NULL;
    dest->used_segments_ring = NULL;
    dest->segment_capacity = 0;
    dest->head = 0;
    dest->tail = 0;
    dest->window_size = 0;
    dest->cur_segment_head = NULL;
    dest->cur_segment_tail = NULL;
    dest->segment_portal_src = NULL;
    dest->segment_portal_dest = NULL;
    dest->full = false;
    dest->free_empty_segments = false;
    dest->reset_popped_values = false;
}

static Errors PQ_add_free_segment_at_begin(PortalQueue* dest, Segment* next)
{
    Segment* new_segment = (Segment*)malloc(sizeof(Segment) + sizeof(uint8_t) * dest->segment_capacity);
    if (!new_segment) {
        return ALLOC_ERROR;
    }

    new_segment->next = next;
    new_segment->prev = NULL;

    if (next) {
        next->prev = new_segment;
    } else {
        dest->free_segments = new_segment;
    }

    return OK;
}

static void PQ_broke_used_segments_ring(PortalQueue* dest)
{
    dest->used_segments_ring->prev->next = NULL;
}

static void PQ_free_segments(Segment* cur)
{
    while (cur) {
        Segment* next = cur->next;
        free(cur);
        cur = next;
    }
}

static Errors PQ_add_used_segment(PortalQueue* dest, Segment* target_as_next)
{
    if (!dest->free_segments) {
        Errors ret_code = PQ_add_free_segment_at_begin(dest, NULL);
        if (ret_code) {
            return ret_code;
        }
    }

    Segment* segment = dest->free_segments;

    // устанавливаем следующий свободный сегмент в качестве входной точки, избавляя от следов взятого сегмента
    // если сегментов больше нет, то free_segments = NULL
    dest->free_segments = dest->free_segments->next;
    if (dest->free_segments) {
        dest->free_segments->prev = NULL;
    }

    bool target_and_ring_was_equal = target_as_next == dest->used_segments_ring;

    // указываем цель для нулевого target, если в ring никого нет, то он segment, иначе первый сегмент в ring
    target_as_next = (target_as_next)? 
        target_as_next
        : ((dest->used_segments_ring)? dest->used_segments_ring : segment);

    // ring будет segment, когда он был NULL (target при этом тоже NULL) или когда был равен target 
    dest->used_segments_ring = (target_and_ring_was_equal)? segment : dest->used_segments_ring;

    Segment* target_prev = target_as_next->prev;

    // присоединяем сегмент к цели
    segment->next = target_as_next;
    target_as_next->prev = segment;

    // если target и сегмент одинаковы, то связи были установлены ранее
    if (target_as_next != segment) {
        target_prev->next = segment;
        segment->prev = target_prev;
    }

    return OK;
}

static void PQ_remove_used_segment(PortalQueue* dest, Segment* segment)
{
    if (segment == segment->next) {
        dest->used_segments_ring = NULL;

    } else {
        dest->used_segments_ring = (dest->used_segments_ring == segment)? segment->next : dest->used_segments_ring;

        // связать сегменты или зациклить один сегмент, когда в цепи есть кто-то еще
        segment->prev->next = segment->next;
        segment->next->prev = segment->prev;
    }

    segment->prev = NULL;
    segment->next = dest->free_segments; // NULL, если других free сегментов нет
    dest->free_segments = segment;
}

Errors PortalQueue_init(
    PortalQueue* dest, 
    size_t segment_capacity, 
    size_t opt_init_capacity, 
    bool free_empty_segments,
    bool reset_popped_values)
{
    if (!dest) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (!segment_capacity) {
        return BAD_ARGUMENT_ZERO;
    }

    opt_init_capacity = opt_init_capacity ? opt_init_capacity - 1 : 0;
    PQ_pre_init(dest, segment_capacity, free_empty_segments, reset_popped_values);
    Errors ret_code = PQ_add_free_segment_at_begin(dest, NULL);

    Segment* cur_segment = dest->free_segments;
    for (size_t i = 0; i < opt_init_capacity; i++) {

        ret_code = PQ_add_free_segment_at_begin(dest, cur_segment);
        if (ret_code != OK) {
            PQ_free_segments(dest->free_segments);
            return ret_code;
        }

        cur_segment = cur_segment->next;
    }

    PQ_add_used_segment(dest, NULL);
    PQ_post_init(dest);

    return OK;
}

Errors PortalQueue_deinit(PortalQueue* dest)
{
    if (!dest) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    PQ_free_segments(dest->free_segments);
    PQ_broke_used_segments_ring(dest);
    PQ_free_segments(dest->used_segments_ring);

    PQ_post_deinit(dest);

    return OK;
}

Errors PortalQueue_push(PortalQueue* dest, uint8_t value)
{
    Errors ret_code = OK;

    if (!dest) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    // когда head оказывается в сегменте с portal, он всегда находится на 0 индексе
    // когда очередь заполнена, то head и tail на одном сегменте 
    if ((dest->cur_segment_head == dest->cur_segment_tail && dest->segment_portal_src) 
        || (dest->cur_segment_head == dest->segment_portal_dest)
        || (dest->full && dest->head == 0)) {

        // добавляем сегмент до head и переносимся туда
        ret_code = PQ_add_used_segment(dest, dest->cur_segment_head);
        if (ret_code != OK) {
            return ret_code;
        }

        dest->cur_segment_head = dest->cur_segment_head->prev;

    } else if (dest->full) {    // когда сегмент заполнен и head не 0
        // создаем портал на старый сегмент для tail
        // создаем новый сегмент после текущего
        // перемещаем head в начало нового сегмента
        dest->segment_portal_src = dest->cur_segment_head;
        dest->segment_portal_dest = dest->cur_segment_head->next;

        ret_code = PQ_add_used_segment(dest, dest->cur_segment_head->next);
        if (ret_code != OK) {
            dest->segment_portal_src = NULL;
            dest->segment_portal_dest = NULL;
            return ret_code;
        }

        dest->window_size = dest->head;
        dest->head = 0;
        dest->cur_segment_head = dest->cur_segment_head->next;
    }

    dest->cur_segment_head->data[dest->head] = value;
    dest->head = (dest->head + 1) % dest->segment_capacity;
    dest->cur_segment_head = (!dest->head)? dest->cur_segment_head->next : dest->cur_segment_head;
    dest->full = (dest->head == dest->tail && dest->cur_segment_head == dest->cur_segment_tail);

    return OK;
}

Errors PortalQueue_pop(PortalQueue* dest, uint8_t* value)
{
    if (!dest) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (!value) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (dest->tail == dest->head && dest->cur_segment_tail == dest->cur_segment_head && !dest->full) {
        return EMPTY_BUFFER;
    }

    *value = dest->cur_segment_tail->data[dest->tail];
    dest->cur_segment_tail->data[dest->tail] = (dest->reset_popped_values)? 0 : dest->cur_segment_tail->data[dest->tail];
    dest->tail = (dest->tail + 1) % dest->segment_capacity;
    dest->full = false;
    
    // cur_segment_tail не бывает NULL до деинициализации, 
    // поэтому проверку segment_portal на NULL можно пропустить
    bool is_on_portal = dest->cur_segment_tail == dest->segment_portal_src;

    // tail вернулся в начало сегмента
    // переход может быть либо через портал, либо самый обычный
    // также освобождается старый сегмент, если там нет head
    if (dest->tail == 0) {

        Segment* prev_segment = dest->cur_segment_tail;

        dest->cur_segment_tail = is_on_portal? dest->segment_portal_dest : dest->cur_segment_tail->next;

        if (dest->free_empty_segments && prev_segment != dest->cur_segment_head) {
            PQ_remove_used_segment(dest, prev_segment);
        }

    // tail достиг сдвига в следующие новые сегменты, 
    // когда он вернулся в этот сегмент с порталом, 
    // сделав 1 круг по старым сегментам через портал,
    // сегмент перестает быть портальным и освобождается, если там нет head
    } else if (is_on_portal && dest->tail == dest->window_size) {

        Segment* prev_segment = dest->cur_segment_tail;

        dest->cur_segment_tail = dest->cur_segment_tail->next;
        dest->tail = 0;
        dest->window_size = dest->segment_capacity;
        dest->segment_portal_src = NULL;
        dest->segment_portal_dest = NULL;

        if (dest->free_empty_segments && prev_segment != dest->cur_segment_head) {
            PQ_remove_used_segment(dest, prev_segment);
        }
    }

    return OK;
}

bool PortalQueue_is_full(PortalQueue* dest)
{
    return dest && dest->full;
}

bool PortalQueue_is_empty(PortalQueue* dest)
{
    return dest && dest->cur_segment_head == dest->cur_segment_tail && dest->head == dest->tail && !dest->full;
}
