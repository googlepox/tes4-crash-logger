#include <format>
#include <set>
#include "GameObjects.h"
#include "GameData.h"
#include "GameMagicEffects.h"
#include "Script.h"
#include "GameTasks.h"
#include "NiExtraData.h"

// If class is described by a single line, no need to name the variable
// If there is a member class, if it's one-line, leave it as one-line, if there are several, prepend the name and add offset

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
		vec.push_back(std::format("ID: {:08X} ({})", refID, refName));
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
		vec.push_back(std::format("ID: {:08X} ({}) : (Plugin: {})", refID, refName, modName));
	}
	return vec;
}

inline auto LogClass(TESObjectREFR& obj)
{
	auto vec = LogClass(static_cast<TESForm&>(obj));
	if (const auto baseForm = obj.TryGetREFRParent())
		vec.append_range(LogMember("\t \t \t \t \t BaseForm:", *baseForm));
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
	for (const auto iter : *TESForm::GetAll())
		if ((iter->eTypeID == TESForm::kType_Creature || iter->eTypeID == TESForm::kType_Character)
			&& reinterpret_cast<Actor*>(iter)->pkBaseProcess == &obj)
			return LogClass(reinterpret_cast<const TESObjectREFR&>(*iter));
	return {};
} 

inline auto LogClass(const NiControllerSequence& obj)
{
	return std::vector{
		SanitizeString(std::string("Name: ") + std::string(obj.m_kName.m_kHandle)),
		SanitizeString(std::string("RootName: ") + std::string(obj.m_kAccumRootName.m_kHandle))
	};
}

inline auto LogClass(const BSAnimGroupSequence& obj)
{
	auto vec = LogClass(static_cast<const NiControllerSequence&>(obj));
	vec.push_back(std::format("AnimGroup: {:04X}", obj.animGroup->animGroup));
	return vec;
}


inline std::vector<std::string> LogClass(const AnimSequenceSingle& obj) { if (obj.pkAnim) return LogClass(*obj.pkAnim); return {}; }

inline std::vector<std::string> LogClass(const AnimSequenceMultiple& obj)
{
	std::vector<std::string> vec;
	UInt32 i = 0;
	for (const auto iter : *obj.pkAnims)
	{
		i++;
		vec.append_range(LogMember(std::format("AnimSequence{}", i), *iter));
	}
	return vec;
} 

inline std::vector<std::string> LogClass(const NiExtraData& obj)
{
	if (const auto name = obj.m_kName.GetStd(); !name.empty())
		return std::vector{ '"' + SanitizeString(name.c_str()) + '"' };
	return {};
} */

inline std::vector<std::string> LogClass(NiExtraData & obj)
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