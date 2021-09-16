/* adlist.h - A generic doubly linked list implementation
 *
 * Copyright (c) 2006-2012, Salvatore Sanfilippo <antirez at gmail dot com>
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

#ifndef __ADLIST_H__
#define __ADLIST_H__

/* Node, List, and Iterator are the only data structures used currently. */
/*双向链表节点*/
typedef struct listNode {
    struct listNode *prev; // 前向节点
    struct listNode *next; // 后向节点
    void *value;           // 节点的值
} listNode;
/*双向链表迭代器*/
typedef struct listIter {
    listNode *next;   // 当前节点
    int direction; // 迭代方向，0正向，1逆向
} listIter;
/*双端链表结构*/
typedef struct list {
    listNode *head; // 表头节点
    listNode *tail; // 表尾节点
    void *(*dup)(void *ptr); // 节点值复制函数
    void (*free)(void *ptr); // 节点值释放函数
    int (*match)(void *ptr, void *key); // 节点值匹配函数
    unsigned long len; // 链表长度
} list;

/* Functions implemented as macros */
#define listLength(l) ((l)->len) // 获取链表长度函数
#define listFirst(l) ((l)->head) // 获取链表表头函数
#define listLast(l) ((l)->tail) // 获取链表表尾函数
#define listPrevNode(n) ((n)->prev) // 获取链表前向节点
#define listNextNode(n) ((n)->next) // 获取链表后向节点
#define listNodeValue(n) ((n)->value) // 获取节点值

#define listSetDupMethod(l,m) ((l)->dup = (m)) // 设置链表拷贝函数
#define listSetFreeMethod(l,m) ((l)->free = (m)) // 设置链表内存释放函数
#define listSetMatchMethod(l,m) ((l)->match = (m)) // 设置链表值匹配函数

#define listGetDupMethod(l) ((l)->dup) // 获取链表拷贝函数
#define listGetFreeMethod(l) ((l)->free) // 获取链表内存释放函数
#define listGetMatchMethod(l) ((l)->match) // 获取链表值匹配函数

/* Prototypes */
list *listCreate(void); // 创建链表
void listRelease(list *list); // 释放链表，会调用ListEmpty（）
void listEmpty(list *list); // 释放链表里所有节点的值内存
list *listAddNodeHead(list *list, void *value); // 创建节点并且增加到表头
list *listAddNodeTail(list *list, void *value); // 创建节点并且增加到表尾
list *listInsertNode(list *list, listNode *old_node, void *value, int after); // 创建节点，并且插入到目标节点的之前或者之后
void listDelNode(list *list, listNode *node); // 从双向链表中删除该节点
listIter *listGetIterator(list *list, int direction); // 获取双向链表迭代器
listNode *listNext(listIter *iter); // 双向链表迭代器获取下一个节点
void listReleaseIterator(listIter *iter);
list *listDup(list *orig);
listNode *listSearchKey(list *list, void *key);
listNode *listIndex(list *list, long index);
void listRewind(list *list, listIter *li);
void listRewindTail(list *list, listIter *li);
void listRotateTailToHead(list *list);
void listRotateHeadToTail(list *list);
void listJoin(list *l, list *o);

/* Directions for iterators */
// 迭代器方向
#define AL_START_HEAD 0 // 从表头向表尾迭代
#define AL_START_TAIL 1 // 从表尾向表头迭代

#endif /* __ADLIST_H__ */
