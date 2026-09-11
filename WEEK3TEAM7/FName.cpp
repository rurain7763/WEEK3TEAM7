#include "FName.h"
#include "TArray.h"
#include "TMap.h"

struct FNamePool
{
	using HashValue64 = uint64;

	struct FNameBlock
	{
		int32 StartIndex;
		int32 Count;
	};

	struct FNameEntry
	{
		FNameBlock ComparisonBlock;
		TArray<FNameBlock> DisplayBlocks;
	};

	HashValue64 HashString(const FString& Str)
	{
		// FNV-1a hash algorithm
		uint64 Hash = 14695981039346656037ull;

		for (unsigned char Ch : Str)
		{
			Hash ^= Ch;
			Hash *= 1099511628211ull;
		}

		return Hash;
	}

	FNameBlock StoreString(const FString& Str)
	{
		FNameBlock Block;
		Block.StartIndex = NameStream.Num();
		Block.Count = Str.Len() + 1;
		NameStream.SetNum(NameStream.Num() + Block.Count);

		memcpy(&NameStream[Block.StartIndex], Str.CStr(), Str.Len() + 1);

		return Block;
	}

	int32 FindComparisonIndex(const FString& ComparisonName, HashValue64 Hash)
	{
		TArray<int32>* EntryIndices = ComparisonNameMap.Find(Hash);
		if (EntryIndices)
		{
			for (int32 Index : *EntryIndices)
			{
				const FNameEntry& Entry = NameEntries[Index];
				if (strncmp(&NameStream[Entry.ComparisonBlock.StartIndex], ComparisonName.CStr(), Entry.ComparisonBlock.Count) == 0)
				{
					return Index;
				}
			}
		}
		
		return -1;
	}

	int32 FindDisplayIndex(int32 ComparisonIndex, const FString& DisplayName)
	{
		const FNameEntry& Entry = NameEntries[ComparisonIndex];

		for (int32 Index = 0; Index < Entry.DisplayBlocks.Num(); ++Index)
		{
			if (strncmp(&NameStream[Entry.DisplayBlocks[Index].StartIndex], DisplayName.CStr(), Entry.DisplayBlocks[Index].Count) == 0)
			{
				return Index;
			}
		}

		return -1;
	}

	int32 StoreComparisionName(const FString& ComparisonName)
	{
		HashValue64 Hash = HashString(ComparisonName);

		int32 ComparisonIndex = FindComparisonIndex(ComparisonName, Hash);
		if (ComparisonIndex == -1)
		{
			FNameBlock ComparisonBlock = StoreString(ComparisonName);
			FNameEntry NewEntry;
			NewEntry.ComparisonBlock = ComparisonBlock;
			ComparisonIndex = NameEntries.Emplace(NewEntry);

			TArray<int32>* EntryIndices = ComparisonNameMap.Find(Hash);
			if (!EntryIndices)
			{
				ComparisonNameMap.Add(Hash, TArray<int32>());
				EntryIndices = ComparisonNameMap.Find(Hash);
			}

			EntryIndices->Add(ComparisonIndex);
		}
		
		return ComparisonIndex;
	}

	int32 StoreDisplayName(int32 ComparisonIndex, const FString& DisplayName)
	{
		int32 DisplayIndex = FindDisplayIndex(ComparisonIndex, DisplayName);
		if (DisplayIndex == -1)
		{
			FNameEntry& Entry = NameEntries[ComparisonIndex];
			FNameBlock DisplayBlock = StoreString(DisplayName);
			DisplayIndex = Entry.DisplayBlocks.Emplace(DisplayBlock);
		}

		return DisplayIndex;
	}

	friend class FName;

	TMap<HashValue64, TArray<int32>> ComparisonNameMap;
	TArray<FNameEntry> NameEntries;
	TArray<char> NameStream;
};

static FNamePool& GetNamePool()
{
	static FNamePool NamePool;
	return NamePool;
}

FName::FName()
	: DisplayIndex(-1)
	, ComparisonIndex(-1)
{
}

FName::FName(const char* pStr)
	: FName(FString(pStr))
{
}

FName::FName(const FString& Name)
{
	FNamePool& NamePool = GetNamePool();

	FString ComparisonName = Name.ToLower();
	
	ComparisonIndex = NamePool.StoreComparisionName(ComparisonName);
	DisplayIndex = NamePool.StoreDisplayName(ComparisonIndex, Name);
}

int32 FName::Compare(const FName& Other) const
{
	return ComparisonIndex - Other.ComparisonIndex;
}

bool FName::operator==(const FName& Other) const
{
	return ComparisonIndex == Other.ComparisonIndex;
}

FString FName::ToString() const
{
	if (!IsValid())
	{
		return FString();
	}

	FNamePool& NamePool = GetNamePool();

	const FNamePool::FNameEntry& Entry = NamePool.NameEntries[ComparisonIndex];
	const FNamePool::FNameBlock& DisplayBlock = Entry.DisplayBlocks[DisplayIndex];

	return FString(&NamePool.NameStream[DisplayBlock.StartIndex]);
}
