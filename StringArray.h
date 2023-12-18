#pragma pragma once

class DataArray 
{
private:
	static const int CAPACITY = 10;
	int _size;
public:
	float data[CAPACITY]{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
	
	DataArray():_size(0){ }
	
	void _clearData()
	{
		for(int i = 0; i < _size ; i++) {data[i] = -1;}
		_size = 0;
	}
	
	float& operator[](int index) 
	{
		return data[index];
	}
	
	void push_back(float value)
	{
		data[_size] = value;
		_size++;
	}
	
	int size() { return _size;};
};

class QueueArray 
{
	static const int CAPACITY = 10;
	int _size;
	int frontIndex; 

public:
	DataArray queue[CAPACITY];
	
	QueueArray(): _size(0), frontIndex(0) {}
	
	DataArray& operator[](int index) 
	{
		return queue[(frontIndex + index) % CAPACITY]; 
	}
	
	DataArray pop()
	{
		DataArray temp = queue[frontIndex];
		queue[frontIndex]._clearData();
		frontIndex = (frontIndex + 1) % CAPACITY;
		_size--;
		return temp;
	}
	
	void push_back(DataArray& array) 
	{
		int backIndex = (frontIndex + _size) % CAPACITY; 
		queue[backIndex] = array;
		_size++;
	}
	
	DataArray pop_back() 
	{
		int backIndex = (frontIndex + _size - 1) % CAPACITY;  
		DataArray temp = queue[backIndex]; 
		queue[backIndex]._clearData(); 
		_size--;  
		return temp;  
	}
	
	void clearQueue() 
	{
		for (int i = 0; i < _size; i++) 
		{
			queue[(frontIndex + i) % CAPACITY]._clearData();
		}
		_size = 0;
		frontIndex = 0;
	}
	
	int size() 
	{
		return _size;
	}
};