#pragma once
#include "winsock2.h"
#include <stdio.h>
#include "Structures.h"


void ExpandQueue(struct Queue* queue) {
	queue->array = (TopicSubscribers*)realloc(queue->array, queue->size * (sizeof(TopicSubscribers)) + sizeof(TopicSubscribers));
	queue->capacity += 1;
}


void ExpandMessageQueue(struct MessageQueue* queue) {
	queue->array = (TopicMessage*)realloc(queue->array, queue->size*(sizeof(TopicMessage)) + sizeof(TopicMessage));
	queue->capacity += 1;
}


struct Queue* CreateQueue(unsigned capacity)
{
	struct Queue* queue = (struct Queue*) malloc(sizeof(struct Queue));
	queue->capacity = capacity;
	queue->front = queue->size = 0;
	queue->rear = capacity - 1;  
	queue->array = (TopicSubscribers*)malloc(queue->capacity * sizeof(TopicSubscribers));
	return queue;
}


struct MessageQueue* CreateMessageQueue(unsigned capacity)
{
	struct MessageQueue* queue = (struct MessageQueue*) malloc(sizeof(struct MessageQueue));
	queue->capacity = capacity;
	queue->front = queue->size = 0;
	queue->rear = capacity - 1;   
	queue->array = (TopicMessage*)malloc(queue->capacity * sizeof(TopicMessage));
	return queue;
}


int IsFull(struct Queue* queue)
{
	return (queue->size == queue->capacity);
}


int IsEmpty(struct Queue* queue)
{
	return (queue->size == 0);
}


int IsFullMessageQueue(struct MessageQueue* queue)
{
	return (queue->size == queue->capacity);
}


int IsEmptyMessageQueue(struct MessageQueue*  queue)
{
	return (queue->size == 0);
}


void Enqueue(struct Queue* queue, char* topic)
{
	TopicSubscribers item;
	strcpy(item.topic, topic);
	item.size = 0;

	if (IsFull(queue))
		ExpandQueue(queue);
	queue->rear = (queue->rear + 1) % queue->capacity;
	queue->array[queue->rear] = item;
	queue->size = queue->size + 1;
	printf("%s Enqueued to queue\n", item.topic);
}


void EnqueueMessageQueue(struct MessageQueue* queue, TopicMessage topic)
{
	if (IsFullMessageQueue(queue))
		ExpandMessageQueue(queue);
	queue->rear = (queue->rear + 1) % queue->capacity;
	queue->array[queue->rear] = topic;
	queue->size = queue->size + 1;
}


TopicSubscribers Dequeue(struct Queue* queue)
{
	if (!IsEmpty(queue)) {
		TopicSubscribers item = queue->array[queue->front];
		queue->front = (queue->front + 1) % queue->capacity;
		queue->size = queue->size - 1;
		return item;
	}
}


TopicMessage DequeueMessageQueue(struct MessageQueue* queue)
{
	if (!IsEmptyMessageQueue(queue)) {
		TopicMessage item = queue->array[queue->front];
		queue->front = (queue->front + 1) % queue->capacity;
		queue->size = queue->size - 1;
		return item;
	}
}
