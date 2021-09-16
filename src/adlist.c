/* adlist.c - A generic doubly linked list implementation
 *
 * Copyright (c) 2006-2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


#include <stdlib.h>
#include "adlist.h"
#include "zmalloc.h"

/* Create a new list. The created list can be freed with
 * listRelease(), but private value of every node need to be freed
 * by the user before to call listRelease(), or by setting a free method using
 * listSetFreeMethod.
 *
 * On error, NULL is returned. Otherwise the pointer to the new list. */
 /*
 * 创建链表
 * 内存分配失败返回NULL
 */
list *listCreate(void)
{
    struct list *list;
    // 分配内存
    if ((list = zmalloc(sizeof(*list))) == NULL)
        return NULL;
    // 初始化属性
    list->head = list->tail = NULL;
    list->len = 0;
    list->dup = NULL;
    list->free = NULL;
    list->match = NULL;
    return list;
}

/* Remove all the elements from the list without destroying the list itself. */
/*
* 释放链表里所有节点的值内存
*/
void listEmpty(list *list)
{
    unsigned long len;
    listNode *current, *next;

    current = list->head;
    len = list->len;
    while(len--) {
        next = current->next;
        if (list->free) list->free(current->value);
        zfree(current);
        current = next;
    }
    list->head = list->tail = NULL;
    list->len = 0;
}

/* Free the whole list.
 *
 * This function can't fail. */
/*
* 释放双向链表，包含所有节点值
*/
void listRelease(list *list)
{
    listEmpty(list);
    zfree(list);
}

/* Add a new node to the list, to head, containing the specified 'value'
 * pointer as value.
 *
 * On error, NULL is returned and no operation is performed (i.e. the
 * list remains unaltered).
 * On success the 'list' pointer you pass to the function is returned. */
/*
* 根据value创建一个节点，双向链表增加该节点到表头
*/
list *listAddNodeHead(list *list, void *value)
{
    // 新增的节点
    listNode *node;
    // 为新节点分配内存
    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    // 设置新节点的值
    node->value = value;
    // 如果链表节点为0，链表头尾指针都指向新节点，新节点前向和后向指针都为NULL
    if (list->len == 0) {
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else { // 链表节点不为空，新节点前置指针为NULL，后向指针指向原先头节点，原先头节点前置指针和链表头指针指向新节点，
        node->prev = NULL;
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }
    // 链表长度+1
    list->len++;
    return list;
}

/* Add a new node to the list, to tail, containing the specified 'value'
 * pointer as value.
 *
 * On error, NULL is returned and no operation is performed (i.e. the
 * list remains unaltered).
 * On success the 'list' pointer you pass to the function is returned. */
 /*
 * 根据value创建一个节点，双向链表添加该节点到表尾
 */
list *listAddNodeTail(list *list, void *value)
{
    listNode *node;

    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;
    if (list->len == 0) {
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {
        node->prev = list->tail;
        node->next = NULL;
        list->tail->next = node;
        list->tail = node;
    }
    list->len++;
    return list;
}
/*
* 根据value创建一个节点，双向链表插入该节点到old_node的之前或者之后
*/
list *listInsertNode(list *list, listNode *old_node, void *value, int after) {
    listNode *node;
    // 分配节点内存
    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    // 设置节点的值
    node->value = value;
    // 插入节点到old的后面
    if (after) {
        node->prev = old_node;
        node->next = old_node->next;
        if (list->tail == old_node) {
            list->tail = node;
        }
    } else { // 插入新节点到old的前面
        node->next = old_node;
        node->prev = old_node->prev;
        if (list->head == old_node) {
            list->head = node;
        }
    }
    if (node->prev != NULL) {
        node->prev->next = node;
    }
    if (node->next != NULL) {
        node->next->prev = node;
    }
    list->len++;
    return list;
}

/* Remove the specified node from the specified list.
 * It's up to the caller to free the private value of the node.
 *
 * This function can't fail. */
 /*
 * 双向链表删除节点,并且释放节点的值和节点内存
 */
void listDelNode(list *list, listNode *node)
{
    // 调整前置节点的指针
    if (node->prev)
        node->prev->next = node->next;
    else
        list->head = node->next;
    // 调整后置节点的指针
    if (node->next)
        node->next->prev = node->prev;
    else
        list->tail = node->prev;
    // 释放节点的值
    if (list->free) list->free(node->value);
    // 释放节点
    zfree(node);
    // 链表长度-1
    list->len--;
}

/* Returns a list iterator 'iter'. After the initialization every
 * call to listNext() will return the next element of the list.
 *
 * This function can't fail. */
/*
* 获取双向链表的迭代器
*
* direction表示迭代器的迭代方向
*      AL_START_HEAD = 0 : 从表头向表尾迭代
*      AL_START_TAIL = 1 : 从表尾向表头迭代
*/
listIter *listGetIterator(list *list, int direction)
{
    listIter *iter;
    // 为迭代器分配内存
    if ((iter = zmalloc(sizeof(*iter))) == NULL) return NULL;
    // 根据迭代方向，设置迭代器起始节点
    if (direction == AL_START_HEAD)
        iter->next = list->head;
    else
        iter->next = list->tail;
    // 记录迭代器方向
    iter->direction = direction;
    return iter;
}

/* Release the iterator memory */
/*
* 释放迭代器
*/
void listReleaseIterator(listIter *iter) {
    zfree(iter);
}

/* Create an iterator in the list private iterator structure */
/*
* 将迭代器的方向设置为 AL_START_HEAD ，
* 并将迭代指针重新指向表头节点。
*/
void listRewind(list *list, listIter *li) {
    li->next = list->head;
    li->direction = AL_START_HEAD;
}
/*
 * 将迭代器的方向设置为 AL_START_TAIL ，
 * 并将迭代指针重新指向表尾节点。
 *
 */
void listRewindTail(list *list, listIter *li) {
    li->next = list->tail;
    li->direction = AL_START_TAIL;
}

/* Return the next element of an iterator.
 * It's valid to remove the currently returned element using
 * listDelNode(), but not to remove other elements.
 *
 * The function returns a pointer to the next element of the list,
 * or NULL if there are no more elements, so the classical usage
 * pattern is:
 *
 * iter = listGetIterator(list,<direction>);
 * while ((node = listNext(iter)) != NULL) {
 *     doSomethingWith(listNodeValue(node));
 * }
 *
 * */
/*
*   获取迭代器的下一节点
*/
listNode *listNext(listIter *iter)
{
    listNode *current = iter->next;

    if (current != NULL) {
        // 根据方向选择下一个节点，保存下一节点
        if (iter->direction == AL_START_HEAD)
            iter->next = current->next;
        else
            iter->next = current->prev;
    }
    return current;
}

/* Duplicate the whole list. On out of memory NULL is returned.
 * On success a copy of the original list is returned.
 *
 * The 'Dup' method set with listSetDupMethod() function is used
 * to copy the node value. Otherwise the same pointer value of
 * the original node is used as value of the copied node.
 *
 * The original list both on success or error is never modified. */
/*
* 复制整个链表
*
* 如果链表有设置值复制函数 dup ，那么对值的复制将使用复制函数进行，
* 否则，新节点将和旧节点共享同一个指针。
*/
list *listDup(list *orig)
{
    list *copy;
    listIter iter;
    listNode *node;
    // 创建空链表
    if ((copy = listCreate()) == NULL)
        return NULL;
    // 复制来源链表的三个函数
    copy->dup = orig->dup;
    copy->free = orig->free;
    copy->match = orig->match;
    // 设置来源链表的迭代器
    listRewind(orig, &iter);
    // 遍历迭代器
    while((node = listNext(&iter)) != NULL) {
        void *value;
        // 存在复制函数,使用复制函数复制节点值
        if (copy->dup) {
            value = copy->dup(node->value);
            if (value == NULL) {
                listRelease(copy);
                return NULL;
            }
        } else
            value = node->value;

          // 将节点添加到链表尾
        if (listAddNodeTail(copy, value) == NULL) {
            listRelease(copy);
            return NULL;
        }
    }
    return copy;
}

/* Search the list for a node matching a given key.
 * The match is performed using the 'match' method
 * set with listSetMatchMethod(). If no 'match' method
 * is set, the 'value' pointer of every node is directly
 * compared with the 'key' pointer.
 *
 * On success the first matching node pointer is returned
 * (search starts from head). If no matching node exists
 * NULL is returned. */
 /*
  * 查找链表 list 中值和 key 匹配的节点。
  *
  * 对比操作由链表的 match 函数负责进行，
  * 如果没有设置 match 函数，
  * 那么直接通过对比值的指针来决定是否匹配。
  *
  * 如果匹配成功，那么第一个匹配的节点会被返回。
  * 如果没有匹配任何节点，那么返回 NULL 。
  *
  * T = O(N)
  */
listNode *listSearchKey(list *list, void *key)
{
    listIter iter;
    listNode *node;
    // 迭代链表
    listRewind(list, &iter);
    while((node = listNext(&iter)) != NULL) {
        // 存在匹配函数则调用
        if (list->match) {
            if (list->match(node->value, key)) {
                return node;
            }
        } else { // 没有匹配函数比较指针
            if (key == node->value) {
                return node;
            }
        }
    }
    return NULL;
}

/* Return the element at the specified zero-based index
 * where 0 is the head, 1 is the element next to head
 * and so on. Negative integers are used in order to count
 * from the tail, -1 is the last element, -2 the penultimate
 * and so on. If the index is out of range NULL is returned. */
/*
 * 根据索引index获取链表节点
 *
 * 索引以 0 为起始，也可以是负数， -1 表示链表最后一个节点，诸如此类。
 *
 * 如果索引超出范围（out of range），返回 NULL 。
 *
 */
listNode *listIndex(list *list, long index) {
    listNode *n;

    if (index < 0) {
        index = (-index)-1;
        n = list->tail;
        while(index-- && n) n = n->prev;
    } else {
        n = list->head;
        while(index-- && n) n = n->next;
    }
    return n;
}

/* Rotate the list removing the tail node and inserting it to the head. */
/*
* 将表尾节点移到表头
*/
void listRotateTailToHead(list *list) {
    if (listLength(list) <= 1) return;

    /* Detach current tail */
    listNode *tail = list->tail;
    list->tail = tail->prev;
    list->tail->next = NULL;
    /* Move it as head */
    list->head->prev = tail;
    tail->prev = NULL;
    tail->next = list->head;
    list->head = tail;
}

/* Rotate the list removing the head node and inserting it to the tail. */
/*
* 将表头节点移到表尾
*/
void listRotateHeadToTail(list *list) {
    if (listLength(list) <= 1) return;

    listNode *head = list->head;
    /* Detach current head */
    list->head = head->next;
    list->head->prev = NULL;
    /* Move it as tail */
    list->tail->next = head;
    head->next = NULL;
    head->prev = list->tail;
    list->tail = head;
}

/* Add all the elements of the list 'o' at the end of the
 * list 'l'. The list 'other' remains empty but otherwise valid. */
/*
* 合并链表,将链表o里的所有节点接到到链表l的尾部
*/
void listJoin(list *l, list *o) {
    if (o->len == 0) return;

    o->head->prev = l->tail;

    if (l->tail)
        l->tail->next = o->head;
    else
        l->head = o->head;

    l->tail = o->tail;
    l->len += o->len;

    /* Setup other as an empty list. */
    o->head = o->tail = NULL;
    o->len = 0;
}
