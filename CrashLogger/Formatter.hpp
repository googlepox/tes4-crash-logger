#pragma once
#include <format>
#include <set>
#include "GameObjects.h"
#include "GameMagicEffects.h"
#include "Script.h"
#include "GameTasks.h"
#include "NiExtraData.h"
#include "CLUtilities.hpp"

// If class is described by a single line, no need to name the variable
// If there is a member class, if it's one-line, leave it as one-line, if there are several, prepend the name and add offset

class BSFile
{
public:
	BSFile();
	~BSFile();

	virtual void	Destructor(bool freeMemory);				// 00
	virtual void	Unk_01(void);								// 04
	virtual void	Unk_02(void);								// 08
	virtual void	Seek(SInt32 offset, UInt32 origin); //(void);								// 0C
	virtual void	Unk_04(void);								// 10
	virtual void	DumpAttributes(NiTArray <char*>* dst);	// 14
	virtual UInt32	GetSize(void);								// 18
	virtual void	Unk_07(void);								// 1C  //Check SkyBSA def.
	virtual void	Unk_08(void);								// 20
	virtual void	Unk_09(void);								// 24
	virtual void	Unk_0A(void);								// 28
	virtual void	Unk_0B(void);								// 2C
	virtual void	Unk_0C(void);								// 30
	virtual void	Unk_Read(void* destination, UInt32 sizeToRead);								// 34
	virtual void	Unk_Write(void);							// 38

	//	void	** m_vtbl;		// 000
	void* m_readProc;	// 004 - function pointer
	void* m_writeProc;	// 008 - function pointer
	UInt32	m_bufSize;		// 00C
	UInt32 lastReadSize; // 010 // return value of fread, fread_s, and friends, when we're reading into our own buffer
	UInt32	m_unk014;		// 014
	void* m_buf;		// 018
	FILE* m_file;		// 01C
	UInt32	m_writeAccess;	// 020
	UInt8	m_good;			// 024
	UInt8	m_pad025[3];	// 025
	UInt8	m_unk028;		// 028
	UInt8	m_pad029[3];	// 029
	UInt32	m_unk02C;		// 02C
	UInt32	m_pos;			// 030
	UInt32	m_unk034;		// 034
	UInt32	m_unk038;		// 038
	char	m_path[0x104];	// 03C
	UInt32	m_unk140;		// 140
	UInt32	m_unk144;		// 144
	UInt32	m_pos2;			// 148 - used if m_pos is 0xFFFFFFFF
	UInt32	m_unk14C;		// 14C
	UInt32	m_fileSize;		// 150
};

struct ModEntry
{
	// 41C / 420
	struct Data		// referred to as 'TESFile' by Bethesda
	{
		enum
		{
			kFlag_IsMaster = 1 << 0,
			kFlag_Loaded = 1 << 2,
			kFlag_Active = 1 << 3
		};

		struct	ChunkInfo
		{
			UInt32	type;		// e.g. 'GRUP', 'GLOB', etc
			UInt32	length;
		};

		struct	RecordInfo
		{
			ChunkInfo		chunkInfo;
			UInt32			flags;
			UInt32			recordID;
			TrackingData	trackingData;
		};
		class GroupInfo : public RecordInfo
		{// size 18/18
			//     /*00*/ RecordInfo    // for group records, the size includes the 14 bytes of the header
			UInt32        recordOffset;   // used internally to track header offsets of all open groups
		};

		struct  SizeInfo
		{
			UInt32		fileSizeHigh;			// WIN32_FIND_DATA::nFileSizeHigh
			UInt32		fileSizeLow;			// WIN32_FIND_DATA::nFileSizeLow
		};

		// static members: B33C1C, B33C20
		typedef BSSimpleList<GroupInfo*> GroupList;

		UInt32	errorState;							// 000 appears to indicate status of file (open, closed, etc) 2, 9, 0C do stuff
		UInt32	ghostFileParent;							// 004
		UInt32	childThreadGhostFiles;							// 008
		BSFile* unkFile00C;							// 00C
		BSFile* bsFile;						// 010
		UInt32	unk014;							// 014
		UInt32	unk018;							// 018
		char	name[0x104];					// 01C
		char	filepath[0x104];				// 120 relative to "Oblivion\"
		UInt32	unk224;							// 224
		UInt32	unk228;							// 228 init to *(0xB055CC), seen 0x2800
		UInt32	unk22C[(0x23C - 0x22C) >> 2];	// 22C
		RecordInfo	currentRecordInfo;					// 23C
		ChunkInfo	currentChunk;					// 250
		UInt32               fileSize; // same as FileSizeLow in find data
		UInt32               currentRecordOffset; // offset of current record in file
		UInt32               currentChunkOffset; // offset of current chunk in record
		UInt32               fetchedChunkDataSize; // number of bytes read in last GetChunkData() call
		GroupInfo            unkFile268; // used when saving empty form records, e.g. for deleted forms
		UInt32               unkFile280; // used when saving empty form records, e.g. for deleted forms  //280
		GroupList            openGroups; // stack of open group records, from lowest level to highest //284
		bool                 headerRead; // set after header has been successfully parsed //28C
		UInt8                padFile28D[3];
		WIN32_FIND_DATA	findData;				// 290
		UInt32	version;						// 3D0 plugin version (0.8/1.0)
		UInt32	formCount;						// 3D4 record/form count
		UInt32	nextFormID;						// 3D8 used by TESFile::sub_486BF0 in the editor
		UInt32	flags;							// 3DC
		tList<char>		masterList;				// 3E0 linked list of .esm dependencies
		tList<SizeInfo>	masterSizeInfo;			// 3E8 linked list of file size info for above list
		UInt32	idx;							// 3F0 //This seems to represent the index of the current file (equals to the size of the master counts )
		void* unk3F4;						// 3F4
		UInt32	unk3F8;							// 3F8
		UInt32	unk3FC;							// 3FC
		UInt8	unk400;							// 400 init to -1
		UInt8	pad401[3];
		BSStringT	authorName;						// 404
		BSStringT	modDescription;					// 40C

		void* currentRecordDCBuffer;		 // buffer for decompressed record data //414
		UInt32 currentRecordDCLength; // length of decompressed record data //418
		//TESFile*             unkFile41C; // file this object was cloned from. used for local copies of network files?  COEF also report this member increasing the max size
	};

	Data* data;
	ModEntry* next;

	ModEntry* Next() const { return next; }
	Data* Info() const { return data; }
	bool IsLoaded()	const { return (data && (data->flags & Data::kFlag_Loaded)) ? true : false; }
};

struct TESRegionDataManager;

class DataHandler
{
public:
	DataHandler();
	~DataHandler();

	struct _Unk8B8											// as seen in the editor
	{
		UInt32				unk00;							// 00
		UInt32				unk04;							// 04
		UInt32				unk08;							// 08 initialized to (numLoadedMods << 24) | 0x800 during plugin load
		ModEntry::Data* activeFile;						// 0C active plugin
	};

	BoundObjectListHead* boundObjects;					// 000
	tList<TESPackage>		packages;						// 004
	tList<TESWorldSpace>	worldSpaces;					// 00C
	tList<TESClimate>		climates;						// 014
	tList<TESWeather>		weathers;						// 01C
	tList<EnchantmentItem>	enchantmentItems;				// 024
	tList<SpellItem>		spellitems;						// 02C
	tList<TESHair>			hairs;							// 034
	tList<TESEyes>			eyes;							// 03C
	tList<TESRace>			races;							// 044
	tList<TESLandTexture>	landTextures;					// 04C
	tList<TESClass>			classes;						// 054
	tList<TESFaction>		factions;						// 05C
	tList<Script>			scripts;						// 064
	tList<TESSound>			sounds;							// 06C
	tList<TESGlobal>		globals;						// 074
	tList<TESTopic>			topics;							// 07C
	tList<TESQuest>			quests;							// 084
	tList<BirthSign>		birthsigns;						// 08C
	tList<TESCombatStyle>	combatStyles;					// 094
	tList<TESLoadScreen>	loadScreens;					// 09C
	tList<TESWaterForm>		waterForms;						// 0A4
	tList<TESEffectShader>	effectShaders;					// 0AC
	tList<TESObjectANIO>	objectAnios;					// 0B4
	TESRegionList* regionList;					// 0BC
	NiTLargeArray <TESObjectCELL*>	cellArray;				// 0C0
	//	UInt32					unk0D0[2];						// 0D0
	TESSkill				skills[0x15];					// 0D8
	_Unk8B8					unk8B8;							// 8B8
	ModEntry				modList;						// 8C8
	UInt32					numLoadedMods;					// 8D0
	ModEntry::Data* modsByID[0xFF];				// 8D4
	UInt32					unkCD0[(0xCD8 - 0xCD0) >> 2];	// CD0
	TESRegionDataManager* regionDataManager;			// CD8
	void* unkCDC;							// CDC //ptr to ExtraData?

	bool	ConstructObject(ModEntry::Data* tesFile, bool unk1);

	const ModEntry* LookupModByName(const char* modName);
	const ModEntry** GetActiveModList();		// returns array of modEntry* corresponding to loaded mods sorted by mod index
	UInt8 GetModIndex(const char* modName);
	UInt8 GetActiveModCount();
	const char* GetNthModName(UInt32 modIndex);
	TESGlobal* GetGlobalVarByName(const char* varName, UInt32 nameLen);
	TESQuest* GetQuestByEditorName(const char* questName, UInt32 nameLen = -1);
	UInt32 GetFormIDBase(const char* modName);
	const char* GetModNameForFormID(UInt32 formID);
};

inline auto Offset(std::vector<std::string> vector)
{
	for (auto i : vector) i.insert(0, "    ");
	return vector;
}

template<class Member> auto LogMember(const std::string& name, Member& member)
{
	std::vector<std::string> vec = LogClass(member);
	if (vec.size() == 1) return std::vector{ name + " " + vec[0] };
	vec = Offset(vec);
	vec.insert(vec.begin(), name);
	vec.insert(vec.begin(), "\t \t \t \t \t ");
	return vec;
}

template<class Member> std::string LogClassLineByLine(Member& member)
{
	std::string output;
	std::vector<std::string> vec = LogClass(member);
	for (const auto& i : vec)
		output += i + '\n';
	return output;
}

inline auto LogClass(TESForm& obj)
{
	std::vector<std::string> vec;
	UInt32 refID = obj.refID;
	UInt32 modIndex = refID >> 24;
	std::string modName;
	std::string refName = obj.GetEditorName();
	std::string  loaded = obj.flags &  TESForm::FormFlags::kFormFlags_Linked ? "" : "<Not Linked>";
	if (modIndex == 0xFF) {
		if (refName.empty())
		{
			if (!&obj) {
				refName = std::format("Temp {} (null baseRef?)", TypeNames[obj.typeID]);
			}
			else {
				refName = std::format("Temp {} ({})", TypeNames[obj.typeID], obj.GetName());
			}
		}
		vec.push_back(std::format("ID: {:08X} ({}) {}", refID, refName, loaded));
	}
	else if (modIndex != 0xFF) {
		std::string modName = (*g_dataHandler)->GetNthModName(modIndex);

		ModEntry::Data* sourceMod = (ModEntry::Data*)obj.modRefList.Info();
		ModRefListVisitor newVisitor = ModRefListVisitor(&obj.modRefList);
		ModEntry::Data* lastMod = (ModEntry::Data*)newVisitor.GetLastNode()->Info();

		if (sourceMod != lastMod) {
			modName = std::format("\"{}\" (Last modified by: \"{}\")", modName, lastMod->name);
		}
		else {
			modName = std::format("\"{}\"", modName);
		}
		vec.push_back(std::format("ID: {:08X} ({}) : (Plugin: {}) {}", refID, refName, modName, loaded));
	}
	return vec;
}

inline auto LogClass(TESObjectREFR& obj)
{
	auto vec = LogClass(static_cast<TESForm&>(obj));
	TESForm* baseForm = obj.baseForm;
	if (baseForm) {
		vec.append_range(LogMember("\t \t \t \t \t BaseForm:", *baseForm));
	}
	else {
		vec.push_back("\t \t \t \t \t BaseForm: NULL");
	}
	return vec;
}

inline auto LogClass(TESPathGrid& obj)
{
	auto vec = LogClass(static_cast<TESForm&>(obj));
	if (obj.theChildCell)
		vec.append_range(LogMember("\t \t \t \t \t Cell:", static_cast<TESForm&>(*obj.theChildCell)));
	return vec;
}

//inline std::vector<std::string> LogClass(const ActorMover& obj) { if (obj.pkActor) return LogClass(*obj.pkActor); return {}; }
inline std::vector<std::string> LogClass(const QueuedReference& obj) { if (obj.refr) return LogClass(*obj.refr); return {}; }


/*
inline std::vector<std::string>  LogClass(const BaseProcess& obj)
{
	for (const auto iter : (*g_dataHandler)->boundObjects)
		if ((iter->eTypeID == TESForm::kType_Creature || iter->eTypeID == TESForm::kType_Character)
			&& reinterpret_cast<Actor*>(iter)->pkBaseProcess == &obj)
			return LogClass(reinterpret_cast<const TESObjectREFR&>(*iter));
	return {};
} */

inline auto LogClass(const NiControllerSequence& obj)
{
	return std::vector{
		//SanitizeString(std::string("Name: ") + std::string(obj.filePath),
		SanitizeString(std::string("File: ") + std::string(obj.filePath))
		//SanitizeString(std::string("RootName: ") + std::string(obj.m_kAccumRootName.m_kHandle))
	};
}

inline auto LogClass(const BSAnimGroupSequence& obj)
{
	auto vec = LogClass(static_cast<const NiControllerSequence&>(obj));
	vec.push_back(std::format("AnimGroup: {:04X}", obj.animGroup->animGroup));
	return vec;
}

inline std::vector<std::string> LogClass(const AnimSequenceSingle& obj) { if (obj.Anim) return LogClass(*obj.Anim); return {}; }

inline std::vector<std::string> LogClass(const AnimSequenceMultiple& obj)
{
	std::vector<std::string> vec;
	UInt32 i = 0;

	for (NiTPointerList<BSAnimGroupSequence>::Node* node = obj.Anims->start; node; node = node->next)
	{
		if (!node->data)
			continue;

		++i;

		auto sub = LogMember(
			std::format("AnimSequence{}", i),
			*node->data
		);

		vec.insert(vec.end(), sub.begin(), sub.end());
	}

	return vec;
}

inline std::vector<std::string> LogClass(const NiExtraData& obj)
{
	if (const auto name = obj.m_pcName; name)
		return std::vector{ '"' + SanitizeString(name) + '"' };
	return {};
}

inline std::vector<std::string> LogClass(NiObjectNET& obj)
{
	const auto name = obj.m_pcName;
	if (name)
		return std::vector{ '"' + SanitizeString(name) + '"' };
	return {};
}

inline std::vector<std::string> LogClass(NiNode& obj)
{
	std::vector<std::string> vec;
	if (const auto name = obj.m_pcName)
		vec = LogMember("Name: ", static_cast<NiObjectNET&>(obj));
	//if (const auto ref = TESObjectREFR::FindReferenceFor3D(&obj))
		//vec.append_range(LogMember("Reference:", *ref));
	return vec;
}

inline std::vector<std::string> LogClass(NiTriStrips& obj)
{
	std::vector<std::string> vec;
	if (const auto name = obj.m_pcName)
		vec = LogMember("Name: ", static_cast<NiObjectNET&>(obj));
	//if (const auto ref = TESObjectREFR::FindReferenceFor3D(&obj))
		//vec.append_range(LogMember("Reference:", *ref));
	return vec;
}

inline std::vector<std::string> LogClass(const BSFile& obj) { return std::vector{ '"' + SanitizeString(obj.m_path) + '"' }; }
inline std::vector<std::string> LogClass(const TESModel& obj) { return std::vector{ '"' + SanitizeString(obj.nifPath.m_data) + '"' }; }


inline std::vector<std::string> LogClass(const QueuedModel& obj)
{
	std::vector<std::string> vec;
	if (obj.filePath)
		vec.push_back(std::string("Path: ") + '"' + SanitizeString(obj.filePath) + '"');
	if (obj.model)
		vec.append_range(LogMember("Model:", *obj.model));
	return vec;
}

inline std::vector<std::string> LogClass(const TESTexture& obj) { return std::vector{ '"' + SanitizeString(obj.ddsPath.m_data) + '"' }; }
//inline std::vector<std::string> LogClass(const QueuedTexture& obj) { return std::vector{ '"' + SanitizeString(obj.pFileName) + '"' }; }
//inline std::vector<std::string> LogClass(const NiStream& obj) { return std::vector{ '"' + SanitizeString(obj.m_acFileName) + '"' }; }
inline std::vector<std::string> LogClass(const ActiveEffect& obj) { if (obj.enchantObject) return LogClass(*obj.enchantObject); return {}; }


inline std::vector<std::string> LogClass(Script& obj)
{
	auto vec = LogClass(static_cast<TESForm&>(obj));
	/*
	if (obj.data && std::string(obj.GetEditorID2()).empty())
	{
		//const auto str = DecompileScriptToFolder(std::format("UnknownScript {:08X}", obj.refID), const_cast<Script*>(&obj), "gek", "Crash Logger");
		//vec.push_back(str);
	} */
	return vec;
}

inline std::vector<std::string> LogClass(const ScriptEffect& obj)
{
	auto vec = LogClass(static_cast<const ActiveEffect&>(obj));
	if (obj.data)
		vec.append_range(LogMember("Script:", *obj.data));
	return vec;
}

inline std::vector<std::string> LogClass(const QueuedKF& obj) { if (obj.kf) return std::vector{ '"' + SanitizeString(obj.kf->path) + '"' }; return {}; }
//inline std::vector<std::string> LogClass(const bhkRefObject& obj) { if (const auto object = obj.hkObj) return LogClass(*object); return {}; }

/*
inline std::vector<std::string> LogClass(const NiCollisionObject& obj)
{
	if (const auto object = obj.m_pkSceneObject) {
		if (object->IsNiNode())
			return LogClass(reinterpret_cast<const NiNode&>(*object));
		return LogClass(*object);
	}
	return {};
}

inline std::vector<std::string> LogClass(const NiTimeController& obj)
{
	if (const auto object = obj.m_pkTarget) {
		if (object->IsNiNode())
			return LogMember("Target:", reinterpret_cast<const NiNode&>(*object));
		return LogMember("Target:", *object);
	}
	return {};
}

inline std::vector<std::string> LogClass(const bhkCharacterController& obj)
{
	if (const auto object = obj.GetNiObject()) {
		if (object->IsNiNode())
			return LogMember("Target:", reinterpret_cast<const NiNode&>(*object));
		return LogMember("Target:", *object);
	}
	return {};
}

inline std::vector<std::string> LogClass(const hkpWorldObject& obj)
{
	std::vector<std::string> vec;
	std::string name = obj.GetName();

	if (!name.empty())
		vec.push_back(std::format("Name: {}", name));

	bhkNiCollisionObject* object = bhkUtilFunctions::GetbhkNiCollisionObject(&obj);
	if (object)
		vec.append_range(LogMember("Collision Object:", reinterpret_cast<const NiCollisionObject&>(*object)));

	return vec;
}

inline std::vector<std::string> LogClass(const IMemoryHeap& obj)
{
	HeapStats stats;
	std::string name = obj.GetName();
	obj.GetHeapStats(&stats, true);
	UInt32 total = stats.uiMemHeapSize;
	UInt32 free = stats.uiMemFreeInBlocks;
	UInt32 used = stats.uiMemUsedInBlocks;
	float percentage = ConvertToMiB(used) / ConvertToMiB(total) * 100.0f;
	std::string str = std::format("{}: {:10}/{:10} ({:.2f}%)", name.c_str(), FormatSize(used), FormatSize(total), percentage);

	return std::vector{ str };
} */
