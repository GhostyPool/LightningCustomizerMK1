#pragma once

enum EFindName
{
	FNAME_Find,
	FNAME_Add,
};


class FName {
public:
	unsigned int Index;
	unsigned int Number;

	FName() : Index(0), Number(0) { }

	bool operator==(const FName& Other) const { return Index == Other.Index && Number == Other.Number; }
	bool operator!=(const FName& Other) const { return Index != Other.Index || Number != Other.Number; }
};
