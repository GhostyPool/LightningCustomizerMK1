#pragma once
#include "FName.h"

class UClass;

class UObject {
public:
	void*		vtable;

	int			Flags;
	int			Index;
	UClass*		Class;
	FName		Name;
	UObject*	Outer;
};


class UClass : public UObject {
public:
	char pad[24];
	UClass* SuperStruct;
};