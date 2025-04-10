#pragma once

#define IS_TYPE(form, type) (*(UInt32*)(form) == kVtbl_##type)
#define NOT_TYPE(form, type) (*(UInt32*)(form) != kVtbl_##type)

enum ObjectVtblOBSE
{
	kVtbl_AlchemyItem = 0xA3217C,
	kVtbl_MagicItemObject = 0xA32364,
	kVtbl_TESIcon = 0xA320A4,
	kVtbl_MagicItem = 0xA322C4,
	kVtbl_TESFullName = 0xA322A0,
	kVtbl_GameSettingCollection = 0xA4049C,
	kVtbl_SettingCollectionList = 0xA404D0,
	kVtbl_EffectItemList = 0xA3282C,
	kVtbl_EffectSetting = 0xA32B14,
	kVtbl_EnchantmentItem = 0xA33C84,
	kVtbl_MagicItemForm = 0xA33DBC,
	kVtbl_IngredientItem = 0xA33F5C,
	kVtbl_SpellItem = 0xA34C3C,
	kVtbl_ExtraCell3D = 0xA34D5C,
	kVtbl_BSExtraData = 0xA34D50,
	kVtbl_ExtraWaterHeight = 0xA34D68,
	kVtbl_ExtraHavok = 0xA34DD4,
	kVtbl_ExtraRegionList = 0xA34D74,
	kVtbl_ExtraCellMusicType = 0xA34D80,
	kVtbl_ExtraCellClimate = 0xA34D8C,
	kVtbl_ExtraCellWaterType = 0xA34D98,
	kVtbl_ExtraProcessMiddleLow = 0xA34DA4,
	kVtbl_ExtraCellCanopyShadowMask = 0xA34DE0,
	kVtbl_ExtraSeenData = 0xA34DB0,
	kVtbl_ExtraNorthRotation = 0xA34DBC,
	kVtbl_ExtraDetachTime = 0xA34DC8,
	kVtbl_BaseExtraList = 0xA34DEC,
	kVtbl_ExtraDataList = 0xA350F8,
	kVtbl_ExtraEditorID = 0xA35140,
	kVtbl_ExtraTeleport = 0xA357DC,
	kVtbl_ExtraOwnership = 0xA35818,
	kVtbl_ExtraGlobal = 0xA35824,
	kVtbl_ExtraRank = 0xA35830,
	kVtbl_ExtraCount = 0xA3583C,
	kVtbl_ExtraHealth = 0xA35848,
	kVtbl_ExtraUses = 0xA35854,
	kVtbl_ExtraTimeLeft = 0xA35860,
	kVtbl_ExtraCharge = 0xA3586C,
	kVtbl_ExtraSoul = 0xA35878,
	kVtbl_ExtraScript = 0xA35884,
	kVtbl_ExtraScale = 0xA35890,
	kVtbl_ExtraHotkey = 0xA358E4,
	kVtbl_ExtraReferencePointer = 0xA35908,
	kVtbl_ExtraTresPassPackage = 0xA35920,
	kVtbl_ExtraLeveledItem = 0xA35938,
	kVtbl_ExtraPoison = 0xA359E0,
	kVtbl_ExtraHeadingTarget = 0xA35A34,
	kVtbl_ExtraHasNoRumors = 0xA35A4C,
	kVtbl_ExtraAnim = 0xA35794,
	kVtbl_ExtraStartingPosition = 0xA35A70,
	kVtbl_ExtraStartingWorldOrCell = 0xA357A0,
	kVtbl_ExtraLight = 0xA357AC,
	kVtbl_ExtraFollower = 0xA357C4,
	kVtbl_ExtraLeveledCreature = 0xA357D0,
	kVtbl_ExtraMapMarker = 0xA357E8,
	kVtbl_ExtraAction = 0xA357F4,
	kVtbl_ExtraContainerChanges = 0xA35800,
	kVtbl_ExtraOriginalReference = 0xA3580C,
	kVtbl_ExtraGhost = 0xA3589C,
	kVtbl_ExtraWorn = 0xA358A8,
	kVtbl_ExtraBoundArmor = 0xA358B4,
	kVtbl_ExtraWornLeft = 0xA358C0,
	kVtbl_ExtraCannotWear = 0xA358CC,
	kVtbl_ExtraInvestmentGold = 0xA358D8,
	kVtbl_ExtraInfoGeneralTopic = 0xA358F0,
	kVtbl_ExtraSeed = 0xA358FC,
	kVtbl_ExtraPackageStartLocation = 0xA35A7C,
	kVtbl_ExtraPackage = 0xA35914,
	kVtbl_ExtraCrimeGold = 0xA3592C,
	kVtbl_ExtraPersistentCell = 0xA35944,
	kVtbl_ExtraRagDollData = 0xA35950,
	kVtbl_ExtraUsedMarkers = 0xA3595C,
	kVtbl_ExtraRunOncePacks = 0xA35968,
	kVtbl_ExtraDistantData = 0xA35974,
	kVtbl_ExtraEnableStateParent = 0xA3598,
	kVtbl_ExtraEnableStateChildren = 0xA35998,
	kVtbl_ExtraRandomTeleportMarker = 0xA359A4,
	kVtbl_ExtraMerchantContainer = 0xA359B0,
	kVtbl_ExtraTravelHorse = 0xA359BC,
	kVtbl_ExtraLevCreaModifier = 0xA359C8,
	kVtbl_ExtraPersuasionPercent = 0xA359D4,
	kVtbl_ExtraLastFinishedSequence = 0xA359EC,
	kVtbl_ExtraXTarget = 0xA359F8,
	kVtbl_ExtraItemDropper = 0xA35A04,
	kVtbl_ExtraDroppedItemList = 0xA35A10,
	kVtbl_ExtraFriendHitList = 0xA35A28,
	kVtbl_ExtraSavedMovement = 0xA35A1C,
	kVtbl_ExtraRefractionProperty = 0xA35A40,
	kVtbl_ExtraEditorRefMoveData = 0xA35A88,
	kVtbl_ExtraSound = 0xA35A58,
	kVtbl_ExtraHaggleAmount = 0xA35A64,
	kVtbl_NiAlphaProperty = 0xA3D604,
	kVtbl_QueuedTexture = 0xA36B38,
	kVtbl_QueuedModel = 0xA36B6C,
	kVtbl_QueuedDistantLOD = 0xA36BA4,
	kVtbl_BSStream = 0xA3690C,
	kVtbl_QueuedTreeBillboard = 0xA36BDC,
	kVtbl_QueuedTreeModel = 0xA36C14,
	kVtbl_QueuedMagicItem = 0xA36C4C,
	kVtbl_QueuedKF = 0xA36C7C,
	kVtbl_QueuedAnimIdle = 0xA36CB0,
	kVtbl_QueuedHead = 0xA36CE4,
	kVtbl_QueuedHelmet = 0xA36D2C,
	kVtbl_QueuedReference = 0xA36D5C,
	kVtbl_AttachDistant3DTask = 0xA3698C,
	kVtbl_IOTask = 0xA36968,
	kVtbl_QueuedTree = 0xA36D9C,
	kVtbl_QueuedCharacter = 0xA36DDC,
	kVtbl_QueuedCreature = 0xA36E1C,
	kVtbl_QueuedPlayer = 0xA36E5C,
	kVtbl_BackgroundCloneThread = 0xA37088,
	kVtbl_INISettingCollection = 0xA404F8,
	kVtbl_LipSynchroSettingCollection = 0xA40548,
	kVtbl_TES = 0xA37470,
	kVtbl_SavedGameFile = 0xA39094,
	kVtbl_BSTempNodeManager = 0xA372EC,
	kVtbl_BSTempNode = 0xA3738C,
	kVtbl_TESActorBaseData = 0xA3B654,
	kVtbl_TESAIForm = 0xA3B718,
	kVtbl_TESAnimation = 0xA3B75C,
	kVtbl_TESAttackDamageForm = 0xA3B770,
	kVtbl_TESAttributes = 0xA3B788,
	kVtbl_TESBipedModelForm = 0xA3B7A8,
	kVtbl_TESContainer = 0xA3B878,
	kVtbl_TESDescription = 0xA3B938,
	kVtbl_TESEnchantableForm = 0xA3B950,
	kVtbl_TESForm = 0xA3BE3C,
	kVtbl_TESHealthForm = 0xA3C02C,
	kVtbl_TESLeveledList = 0xA3C078,
	kVtbl_TESModel = 0xA3C24C,
	kVtbl_TESModelList = 0xA3C28C,
	kVtbl_TESProduceForm = 0xA3C2A0,
	kVtbl_TESQualityForm = 0xA3C35C,
	kVtbl_TESRaceForm = 0xA3C370,
	kVtbl_TESReactionForm = 0xA3C41C,
	kVtbl_TESScriptableForm = 0xA3C490,
	kVtbl_TESSpellList = 0xA3C540,
	kVtbl_TESTexture = 0xA3C650,
	kVtbl_TESUsesForm = 0xA3C66C,
	kVtbl_TESValueForm = 0xA3C680,
	kVtbl_TESWeightForm = 0xA3C6A0,
	kVtbl_AnimSequenceSingle = 0xA3C72C,
	kVtbl_AnimSequenceBase = 0xA3C6E4,
	kVtbl_AnimSequenceMultiple = 0xA3C758,
	kVtbl_NiRefObject = 0xA3FCF8,
	kVtbl_BSAnimGroupSequence = 0xA3F7D4,
	kVtbl_BSTexturePalette = 0xA3FD80,
	kVtbl_NiStencilProperty = 0xA3D744,
	kVtbl_BSCullingProcess = 0xA68CF4,
	kVtbl_hkpAllRayHitCollector = 0xA563EC,
	kVtbl_bhkUnaryAction = 0xA3D49C,
	kVtbl_bhkAction = 0xA3D414,
	kVtbl_bhkSerializable = 0xA3D38C,
	kVtbl_GridArray = 0xA3D718,
	kVtbl_GridCellArray = 0xA3D79C,
	kVtbl_GridDistantArray = 0xA3D7E0,
	kVtbl_TESRegion = 0xA3FEE4,
	kVtbl_TESRegionData = 0xA3FFCC,
	kVtbl_TESRegionDataGrass = 0xA3FFF0,
	kVtbl_TESRegionDataLandscape = 0xA40054,
	kVtbl_TESRegionDataManager = 0xA400A0,
	kVtbl_TESRegionDataMap = 0xA400DC,
	kVtbl_TESRegionDataSound = 0xA40110,
	kVtbl_TESRegionDataWeather = 0xA40140,
	kVtbl_TESRegionGrassObject = 0xA401B8,
	kVtbl_TESRegionGrassObjectList = 0xA40344,
	kVtbl_TESRegionList = 0xA4034C,
	kVtbl_TESAmmo = 0xA4068C,
	kVtbl_TESCombatStyle = 0xA407F4,
	kVtbl_DefaultCombatStyle = 0xA4109C,
	kVtbl_TESEffectShader = 0xA419EC,
	kVtbl_TESFlora = 0xA41C40,
	kVtbl_TESFurniture = 0xA41D1C,
	kVtbl_TESGrass = 0xA4280C,
	kVtbl_TESKey = 0xA42A4C,
	kVtbl_TESLevCreature = 0xA42B9C,
	kVtbl_TESLevItem = 0xA42DC4,
	kVtbl_TESLevSpell = 0xA42F1C,
	kVtbl_TESObjectLIGH = 0xA4319C,
	kVtbl_TESBoundObject = 0xA4385C,
	kVtbl_TESBoundAnimObject = 0xA43984,
	kVtbl_TESBoundTreeObject = 0xA43AAC,
	kVtbl_TESObject = 0xA43644,
	kVtbl_TESObjectExtraData = 0xA4379C,
	kVtbl_TESObjectACTI = 0xA43CB4,
	kVtbl_TESObjectANIO = 0xA43DFC,
	kVtbl_TESObjectAPPA = 0xA43FB4,
	kVtbl_TESObjectARMO = 0xA441BC,
	kVtbl_TESObjectBOOK = 0xA44454,
	kVtbl_TESObjectCLOT = 0xA44644,
	kVtbl_TESObjectCONT = 0xA44824,
	kVtbl_TESObjectDOOR = 0xA44A54,
	kVtbl_TESObjectMISC = 0xA44CA4,
	kVtbl_TESObjectSTAT = 0xA44DEC,
	kVtbl_TESObjectTREE = 0xA44FB4,
	kVtbl_TESModelTree = 0xA44F1,
	kVtbl_TESIconTree = 0xA44F30,
	kVtbl_NiBillboardNode = 0xA45134,
	kVtbl_TESObjectWEAP = 0xA45270,
	kVtbl_TESSigilStone = 0xA45534,
	kVtbl_TESSoulGem = 0xA456F4,
	kVtbl_TESSubspace = 0xA4592C,
	kVtbl_DistantLODLoaderTask = 0xA45A68,
	kVtbl_ExteriorCellLoaderTask = 0xA45BE8,
	kVtbl_TESClimate = 0xA45C9C,
	kVtbl_TESObjectLAND = 0xA46064,
	kVtbl_TESChildCell = 0xA45E54,
	kVtbl_BSPackedAdditionalGeometryData = 0xA45F1C,
	kVtbl_NiAdditionalGeometryData = 0xA45EC4,
	kVtbl_hkReferencedObject = 0xA45E64,
	kVtbl_hkBaseObject = 0xA45E5C,
	kVtbl_NiBinaryExtraData = 0xA45F74,
	kVtbl_hkMoppCode = 0xA45FEC,
	kVtbl_TESLandTexture = 0xA4656C,
	kVtbl_TESObjectCELL = 0xA469D4,
	kVtbl_hkpEntityListener = 0xA46684,
	kVtbl_TESObjectREFR = 0xA46C44,
	kVtbl_TESPathgrid = 0xA47644,
	kVtbl_TESRoad = 0xA4788C,
	kVtbl_bhkRigidBody = 0xA5605C,
	kVtbl_bhkEntity = 0xA55DFC,
	kVtbl_bhkCollisionObject = 0xA55FCC,
	kVtbl_bhkSimpleShapePhantom = 0xA55CBC,
	kVtbl_bhkShapePhantom = 0xA55C2C,
	kVtbl_bhkPhantom = 0xA55B9C,
	kVtbl_bhkConvexShape = 0xA55AFC,
	kVtbl_bhkSphereRepShape = 0xA55A5C,
	kVtbl_bhkShape = 0xA5591C,
	kVtbl_bhkSphereShape = 0xA55D4C,
	kVtbl_TESWaterForm = 0xA47F0C,
	kVtbl_TESWeather = 0xA47FEC,
	kVtbl_TESWorldSpace = 0xA4829C,
	kVtbl_hkRayHitCollector = 0xA48DD0,
	kVtbl_NiTArray = 0xA35CF4,
	kVtbl_TESSoundFile = 0xA52168,
	kVtbl_hkpRayHitCollector = 0xA48DD0,
	kVtbl_TESGlobal = 0xA4981C,
	kVtbl_TESLoadScreen = 0xA4997C,
	kVtbl_Script = 0xA49DA4,
	kVtbl_TESSound = 0xA5219C,
	kVtbl_BirthSign = 0xA52524,
	kVtbl_SoundCollisionListener = 0xA2FE0C,
	kVtbl_TESAnimGroup = 0xA529CC,
	kVtbl_TESClass = 0xA52C6C,
	kVtbl_TESCreature = 0xA5324C,
	kVtbl_TESActorBase = 0xA52F84,
	kVtbl_TESEyes = 0xA53414,
	kVtbl_TESFaction = 0xA53524,
	kVtbl_TESHair = 0xA53654,
	kVtbl_TESIdleForm = 0xA53774,
	kVtbl_TESModelAnim = 0xA53734,
	kVtbl_TESNPC = 0xA53DD4,
	kVtbl_TESQuest = 0xA541EC,
	kVtbl_TESRace = 0xA548FC,
	kVtbl_TESSkill = 0xA54EFC,
	kVtbl_TESTopic = 0xA5555C,
	kVtbl_TopicInfoArray = 0xA554D4,
	kVtbl_TESTopicInfo = 0xA5568C,
	kVtbl_bhkEntityListener = 0xA562D4,
	kVtbl_hkpAllCdPointCollector = 0xA55DEC,
	kVtbl_hkpCdPointCollector = 0xA559BC,
	kVtbl_CellMopp = 0xA56114,
	kVtbl_bhkTriSampledHeightFieldBvTreeShape = 0xA55F2C,
	kVtbl_bhkBvTreeShape = 0xA55E8C,
	kVtbl_HavokStreambufFactory = 0xA561E0,
	kVtbl_HavokError = 0xA56214,
	kVtbl_hkClosestRayHitCollector = 0xA562BC,
	kVtbl_HavokFileStreambufReader = 0xA5614C,
	kVtbl_HavokStreambufWriter = 0xA561B4,
	kVtbl_SpecificItemCollector = 0xA562C8,
	kVtbl_TESArrowTargetListener = 0xA562F4,
	kVtbl_hkpAllCdBodyPairCollector = 0xA56370,
	kVtbl_hkpCdBodyPairCollector = 0xA56314,
	kVtbl_TESTrapListener = 0xA56350,
	kVtbl_hkpPhantomListener = 0xA56320,
	kVtbl_TESWaterListener = 0xA56384,
	kVtbl_BlendSettingCollection = 0xA566A4,
	kVtbl_Atmosphere = 0xA56B40,
	kVtbl_Clouds = 0xA56BE0,
	kVtbl_Moon = 0xA56CAC,
	kVtbl_Precipitation = 0xA56D7C,
	kVtbl_Sky = 0xA56E14,
	kVtbl_SkyObject = 0xA570C8,
	kVtbl_SkyTask = 0xA570E4,
	kVtbl_Stars = 0xA57144,
	kVtbl_Sun = 0xA571DC,
	kVtbl_NiNode = 0xA7E38C,
	kVtbl_NiTriStrips = 0xA7F27C,
	kVtbl_NiStream = 0xA7EA1C,
	kVtbl_NiObjectNET = 0xA7D5BC,
	kVtbl_NiExtraData = 0xA7F6DC,
	kVtbl_BSFaceGenAnimationData = 0xA6424C,
	kVtbl_BSFaceGenImage = 0xA64488,
	kVtbl_BSFaceGenKeyframe = 0xA644AC,
	kVtbl_BSFaceGenKeyframeMultiple = 0xA644FC,
	kVtbl_BSFaceGenModelMap = 0xA64820,
	kVtbl_BSFaceGenModel = 0xA64A98,
	kVtbl_BSFaceGenModelExtraData = 0xA64A3C,
	kVtbl_BSFaceGenMorph = 0xA64C38,
	kVtbl_BSFaceGenMorphStatistical = 0xA64C58,
	kVtbl_BSFaceGenMorphDataHead = 0xA64CCC,
	kVtbl_BSFaceGenMorphDataHair = 0xA64D20,
	kVtbl_BSFaceGenBaseMorphExtraData = 0xA64C6C,
	kVtbl_BSFaceGenNiNode = 0xA64F5C,
	kVtbl_BSTreeModel = 0xA654D4,
	kVtbl_BSTreeNode = 0xA65854,
	kVtbl_bhkSPCollisionObject = 0xA655CC,
	kVtbl_bhkPCollisionObject = 0xA65544,
	kVtbl_bhkMultiSphereShape = 0xA65654,
	kVtbl_bhkTransformShape = 0xA656F4,
	kVtbl_bhkCapsuleShape = 0xA65794,
	kVtbl_TESPackage = 0xA6743C,
	kVtbl_NiTask = 0xA680DC,
	kVtbl_BSTECreateTask = 0xA6813C,
	kVtbl_BSTempEffect = 0xA681AC,
	kVtbl_BSTempEffectDecal = 0xA6822C,
	kVtbl_BSTempEffectGeometryDecal = 0xA6851C,
	kVtbl_NiSkinInstance = 0xA684CC,
	kVtbl_BSTempEffectParticle = 0xA686CC,
	kVtbl_hkpEntityActivationListener = 0xA752F0,
	kVtbl_TerrainLodQuadLoadTask = 0xA47CE0,
	kVtbl_TileMenu = 0xA6AF14,
	kVtbl_TileWindow = 0xA6AF3C,
	kVtbl_TileRect = 0xA69138,
	kVtbl_Tile = 0xA690E8,
	kVtbl_TileImage = 0xA69110,
	kVtbl_IconArray = 0xA6BED8,
	kVtbl_AlchemyMenu = 0xA6B1B4,
	kVtbl_AudioMenu = 0xA6B29C,
	kVtbl_StatsMenu = 0xA6D9DC,
	kVtbl_TextEditMenu = 0xA6A04C,
	kVtbl_VideoMenu = 0xA6A09C,
	kVtbl_VideoDisplayMenu = 0xA6DCCC,
	kVtbl_ControlsMenu = 0xA69FF4,
	kVtbl_TrainingMenu = 0xA6DC74,
	kVtbl_QuantityMenu = 0xA6D15C,
	kVtbl_QuickKeysMenu = 0xA6D1F4,
	kVtbl_BookMenu = 0xA6B334,
	kVtbl_BreathMenu = 0xA6B404,
	kVtbl_ClassMenu = 0xA6B49C,
	kVtbl_ContainerMenu = 0xA6B574,
	kVtbl_CreditsMenu = 0xA6B784,
	kVtbl_DialogMenu = 0xA6B88C,
	kVtbl_EffectSettingMenu = 0xA6B92C,
	kVtbl_EnchantmentMenu = 0xA6BA0C,
	kVtbl_GameplayMenu = 0xA6BD0C,
	kVtbl_GenericMenu = 0xA6BD9C,
	kVtbl_HUDInfoMenu = 0xA6BDF4,
	kVtbl_HUDMainMenu = 0xA6BEE4,
	kVtbl_HUDReticle = 0xA6C004,
	kVtbl_HUDSubtitleMenu = 0xA6C0DC,
	kVtbl_InventoryMenu = 0xA6C18C,
	kVtbl_LevelUpMenu = 0xA6C264,
	kVtbl_LoadingMenu = 0xA6C40C,
	kVtbl_LoadingGameMenu = 0xA6C6BC,
	kVtbl_LockpickMenu = 0xA6C73C,
	kVtbl_MagicMenu = 0xA6C974,
	kVtbl_MagicPopupMenu = 0xA6CA54,
	kVtbl_MainMenu = 0xA6CB1C,
	kVtbl_MapMenu = 0xA6CC9C,
	kVtbl_MessageMenu = 0xA6CE5C,
	kVtbl_NegotiateMenu = 0xA6CF0C,
	kVtbl_OptionsMenu = 0xA6CFAC,
	kVtbl_PauseMenu = 0xA6D03C,
	kVtbl_PersuasionMenu = 0xA6D0CC,
	kVtbl_RaceSexMenu = 0xA6D38C,
	kVtbl_RechargeMenu = 0xA6D45C,
	kVtbl_RepairMenu = 0xA6D5B4,
	kVtbl_SaveMenu = 0xA6D624,
	kVtbl_SigilStoneMenu = 0xA6D664,
	kVtbl_SkillsMenu = 0xA6D71C,
	kVtbl_SleepWaitMenu = 0xA6D7C4,
	kVtbl_SpellMakingMenu = 0xA6D864,
	kVtbl_SpellPurchaseMenu = 0xA6D8CC,
	kVtbl_AbsorbEffect = 0xA75044,
	kVtbl_ActiveEffect = 0xA75094,
	kVtbl_AssociatedItemEffect = 0xA751D4,
	kVtbl_bhkCharacterListenerSpell = 0xA75218,
	kVtbl_bhkTelekinesisListener = 0xA75328,
	kVtbl_BoundItemEffect = 0xA75344,
	kVtbl_CalmEffect = 0xA753A4,
	kVtbl_ChameleonEffect = 0xA753F4,
	kVtbl_CommandCreatureEffect = 0xA75444,
	kVtbl_CommandEffect = 0xA7548C,
	kVtbl_CommandHumanoidEffect = 0xA754D4,
	kVtbl_CureEffect = 0xA7551C,
	kVtbl_DarknessEffect = 0xA75564,
	kVtbl_DemoralizeEffect = 0xA755B4,
	kVtbl_DetectLifeEffect = 0xA755FC,
	kVtbl_DisintegrateArmorEffect = 0xA7564C,
	kVtbl_DisintegrateWeaponEffect = 0xA756C4,
	kVtbl_DispelEffect = 0xA7573C,
	kVtbl_FrenzyEffect = 0xA75784,
	kVtbl_InvisibilityEffect = 0xA757D4,
	kVtbl_LightEffect = 0xA75824,
	kVtbl_LockEffect = 0xA758F4,
	kVtbl_MagicBallProjectile = 0xA75944,
	kVtbl_MagicBoltProjectile = 0xA75BC4,
	kVtbl_MagicCaster = 0xA75DEC,
	kVtbl_MagicFogProjectile = 0xA75EFC,
	kVtbl_MagicHitEffect = 0xA76124,
	kVtbl_MagicModelHitEffect = 0xA761C4,
	kVtbl_MagicProjectile = 0xA76284,
	kVtbl_MagicShaderHitEffect = 0xA764B4,
	kVtbl_MagicSprayProjectile = 0xA76594,
	kVtbl_MagicTarget = 0xA767BC,
	kVtbl_NightEyeEffect = 0xA768A4,
	kVtbl_NonActorMagicCaster = 0xA7693,
	kVtbl_NonActorMagicTarget = 0xA76978,
	kVtbl_OpenEffect = 0xA76984,
	kVtbl_ParalysisEffect = 0xA769CC,
	kVtbl_ReanimateEffect = 0xA76A1C,
	kVtbl_ScriptEffect = 0xA76AAC,
	kVtbl_ShieldEffect = 0xA76AF4,
	kVtbl_SoulTrapEffect = 0xA76B44,
	kVtbl_SummonCreatureEffect = 0xA76BAC,
	kVtbl_SunDamageEffect = 0xA76C54,
	kVtbl_TelekinesisEffect = 0xA76C9C,
	kVtbl_TurnUndeadEffect = 0xA76CEC,
	kVtbl_ValueModifierEffect = 0xA76D34,
	kVtbl_VampirismEffect = 0xA76D84,
	kVtbl_ChangesMap = 0xA3A2EC,
	kVtbl_InteriorCellNewReferencesMap = 0xA3A310,
	kVtbl_ExteriorCellNewReferencesMap = 0xA3A330,
	kVtbl_NumericIDBufferMap = 0xA3A350,
	kVtbl_RegSettingCollection = 0xA40590,
	kVtbl_SceneGraph = 0xA31BD4,
	kVtbl_SeenData = 0xA3201C,
	kVtbl_IntSeenData = 0xA32034,
	kVtbl_Actor = 0xA6E074,
	kVtbl_ArrowProjectile = 0xA6F08C,
	kVtbl_BaseProcess = 0xA6F464,
	kVtbl_bhkCharacterListenerArrow = 0xA6FA54,
	kVtbl_BSDoorHavokController = 0xA6FAF4,
	kVtbl_BSPlayerDistanceCheckController = 0xA6FB84,
	kVtbl_Character = 0xA6FC9C,
	kVtbl_Creature = 0xA710F4,
	kVtbl_HighProcess = 0xA71814,
	kVtbl_LipTask = 0xA71FFC,
	kVtbl_LowProcess = 0xA720A4,
	kVtbl_MiddleHighProcess = 0xA72684,
	kVtbl_MiddleLowProcess = 0xA72D64,
	kVtbl_MobileObject = 0xA732FC,
	kVtbl_PlayerCharacter = 0xA73A0C,
	kVtbl_CombatController = 0xA70A14,
	kVtbl_hkConstraintCinfo = 0xA56484,
	kVtbl_AlarmPackage = 0xA6EF94,
	kVtbl_DialoguePackage = 0xA7151C,
	kVtbl_FleePackage = 0xA71664,
	kVtbl_SpectatorPackage = 0xA746AC,
	kVtbl_TrespassPackage = 0xA747BC,
	kVtbl_AStarNodeList = 0xA748D4,
	kVtbl_AStarWorldNodeList = 0xA749A4,
	kVtbl_PathBuilder = 0xA74C0C,
	kVtbl_DebugTextExtraData = 0xA687BC,
	kVtbl_Menu = 0xA696DC,
	kVtbl_NiVertexColorProperty = 0xA2FD04,
	kVtbl_NiZBufferProperty = 0xA2FDB4,
	kVtbl_NiZWireframeProperty = 0xA2FD5C,
	kVtbl_NiPointLight = 0xA43044,
	kVtbl_NiFloatExtraData = 0xA3F9E4,
	kVtbl_FadeNodeMaxAlphaExtraData = 0xA3FA3C,
	kVtbl_BSScissorNode = 0xA3FB54,
	kVtbl_NiD3DGlobalConstantEntry = 0xA8ACC8,
	kVtbl_NiFile = 0xA36354,
	kVtbl_NiBinaryStream = 0xA853E4,
	kVtbl_NiSearchPath = 0xA853C4,
	kVtbl_NiMemStream = 0xA853FC,
	kVtbl_NiLogBehavior = 0xA8540C,
	kVtbl_NiMemoryLogBehavior = 0xA85414,
	kVtbl_NiProfileLogBehavior = 0xA8541C,
	kVtbl_BSThread = 0xA3D0F0,
	kVtbl_MemoryHeap = 0xA2F810,
	kVtbl_Archive = 0xA35D74,
	kVtbl_ArchiveFile = 0xA35DE4,
	kVtbl_CompressedArchiveFile = 0xA35E64,
	kVtbl_BackgroundLoaderThread = 0xA3631C,
	kVtbl_BackgroundLoader = 0xA36328,
	kVtbl_BSFile = 0xA3636C,
	kVtbl_BSSearchPath = 0xA363E8,
	kVtbl_CTreeEngine = 0xA8C894,
	kVtbl_CIdvCamera = 0xA8C63C,
	kVtbl_Uniform = 0xA8C600,
	kVtbl_PosGen = 0xA8C9B4,
	kVtbl_Normal = 0xA8C9DC,
	kVtbl_BSShaderAccumulator = 0xA8CC5C,
	kVtbl_SkyShader = 0xA8F654,
	kVtbl_HDRShader = 0xA8FFAC,
	kVtbl_BSFadeNode = 0xA3F944,
	kVtbl_DistantLODShaderProperty = 0xA8D1D4,
	kVtbl_ShadowSceneNode = 0xA904B4,
	kVtbl_SkyShaderProperty = 0xA903D4,
	kVtbl_BSFogProperty = 0xA90584,
	kVtbl_BSClearZNode = 0xA3F894,
	kVtbl_NiParticleSystem = 0xA85B0C,
	kVtbl_NiPSysEmitterCtlr = 0xA862C4,
	kVtbl_NiPSysMeshEmitter = 0xA85BCC,
	kVtbl_NiPSysCylinderEmitter = 0xA85D7C,
	kVtbl_NiPSysSphereEmitter = 0xA85DF4,
	kVtbl_NiPSysBoxEmitter = 0xA85E64,
	kVtbl_NiPSysRotationModifier = 0xA85F34,
	kVtbl_NiPSysMeshUpdateModifier = 0xA85ED4,
	kVtbl_NiMeshParticleSystem = 0xA860D4,
	kVtbl_NiPSysEmitter = 0xA8617C,
	kVtbl_NiPSysGravityModifier = 0xA8637C,
	kVtbl_NiPSysSpawnModifier = 0xA86454,
	kVtbl_NiPSysData = 0xA877F4,
	kVtbl_NiPSysUpdateTask = 0xA87E94,
	kVtbl_NiPSysVortexFieldModifier = 0xA8663C,
	kVtbl_NiPSysUpdateCtlr = 0xA8669C,
	kVtbl_NiPSysTurbulenceFieldModifier = 0xA86714,
	kVtbl_NiPSysSphericalCollider = 0xA86784,
	kVtbl_NiPSysResetOnLoopCtlr = 0xA867F4,
	kVtbl_NiPSysRadialFieldModifier = 0xA8686C,
	kVtbl_NiPSysPositionModifier = 0xA868DC,
	kVtbl_NiPSysPlanarCollider = 0xA8694C,
	kVtbl_NiPSysModifierActiveCtlr = 0xA869BC,
	kVtbl_NiPSysInitialRotSpeedVarCtlr = 0xA86A74,
	kVtbl_NiPSysInitialRotSpeedCtlr = 0xA86B2C,
	kVtbl_NiPSysInitialRotAngleVarCtlr = 0xA86BE4,
	kVtbl_NiPSysInitialRotAngleCtlr = 0xA86C9C,
	kVtbl_NiPSysGrowFadeModifier = 0xA86D54,
	kVtbl_NiPSysGravityStrengthCtlr = 0xA86DEC,
	kVtbl_NiPSysGravityFieldModifier = 0xA86EA4,
	kVtbl_NiPSysFieldMaxDistanceCtlr = 0xA86F04,
	kVtbl_NiPSysFieldMagnitudeCtlr = 0xA86FBC,
	kVtbl_NiPSysFieldAttenuationCtlr = 0xA87074,
	kVtbl_NiPSysEmitterSpeedCtlr = 0xA8712C,
	kVtbl_NiPSysEmitterPlanarAngleVarCtlr = 0xA871E4,
	kVtbl_NiPSysEmitterPlanarAngleCtlr = 0xA8729C,
	kVtbl_NiPSysEmitterLifeSpanCtlr = 0xA87354,
	kVtbl_NiPSysEmitterInitialRadiusCtlr = 0xA8740C,
	kVtbl_NiPSysEmitterDeclinationVarCtlr = 0xA874C4,
	kVtbl_NiPSysEmitterDeclinationCtlr = 0xA8757C,
	kVtbl_NiPSysEmitterCtlrData = 0xA87634,
	kVtbl_NiPSysDragModifier = 0xA876DC,
	kVtbl_NiPSysDragFieldModifier = 0xA8777C,
	kVtbl_NiPSysColorModifier = 0xA878AC,
	kVtbl_NiPSysColliderManager = 0xA8790C,
	kVtbl_NiPSysBoundUpdateModifier = 0xA8796C,
	kVtbl_NiPSysBombModifier = 0xA879DC,
	kVtbl_NiPSysAgeDeathModifier = 0xA87AAC,
	kVtbl_NiPSysAirFieldSpreadCtlr = 0xA87B2C,
	kVtbl_NiPSysAirFieldModifier = 0xA87BE4,
	kVtbl_NiPSysAirFieldInheritVelocityCtlr = 0xA87CB4,
	kVtbl_NiPSysAirFieldAirFrictionCtlr = 0xA87D6C,
	kVtbl_NiMeshPSysData = 0xA87E2C,
	kVtbl_NiPSysModifierCtlr = 0xA87F04,
	kVtbl_NiPSysModifier = 0xA8653C,
	kVtbl_NiPSysVolumeEmitter = 0xA865B4,
	kVtbl_NiPSysFieldModifier = 0xA87FDC,
	kVtbl_NiPSysCollider = 0xA88094,
	kVtbl_NiPSysModifierBoolCtlr = 0xA88134,
	kVtbl_NiPSysModifierFloatCtlr = 0xA88204,
	kVtbl_MessageHandler = 0xA3DA08,
	kVtbl_QueuedChildren = 0xA36ACC,
	kVtbl_QueuedFile = 0xA36AD4,
	kVtbl_QueuedFileEntry = 0xA36B04,
	kVtbl_IOManager = 0xA365AC,
	kVtbl_BSTaskThread = 0xA363FC,
	kVtbl_FileFinder = 0xA36414,
	kVtbl_BSScissorTriShape = 0xA3FC04,
	kVtbl_hkCollisionListener = 0xA2FC50,
	kVtbl_hkpBroadPhaseCastCollector = 0xA74C74,
	kVtbl_PathHigh = 0xA74CA4,
	kVtbl_PathLow = 0xA74EDC,
	kVtbl_PathMiddleHigh = 0xA74F3C,
	kVtbl_bhkCharacterListener = 0xA6FA38,
	kVtbl_TESWindListener = 0xA4699C,
	kVtbl_WeaponObject = 0xA565D4,
	kVtbl_bhkLimitedHingeConstraint = 0xA5653C,
	kVtbl_hkLimitedHingeConstraintCinfo = 0xA56658,
	kVtbl_bhkWorldObject = 0xA559CC,
	kVtbl_hkpBoxShape = 0xA6593C,
	kVtbl_hkpAabbPhantom = 0xA75E64,
	kVtbl_NiDX9SourceTextureData = 0xA883C4,
	kVtbl_NiDX9Renderer = 0xA88EA4,
	kVtbl_NiDX9ShaderDeclaration = 0xA89C74,
	kVtbl_NiD3DShaderDeclaration = 0xA89BD4,
	kVtbl_NiD3DDefaultShader = 0xA896EC,
	kVtbl_NiD3DShader = 0xA8977C,
	kVtbl_NiDX92DBufferData = 0xA8981C,
	kVtbl_NiDX9TextureBufferData = 0xA898E4,
	kVtbl_NiDX9ImplicitBufferData = 0xA8997C,
	kVtbl_NiDX9ImplicitDepthStencilBufferData = 0xA899F4,
	kVtbl_NiDX9AdditionalDepthStencilBufferData = 0xA89A4C,
	kVtbl_NiD3DPass = 0xA882DC,
	kVtbl_NiD3DShaderInterface = 0xA8A7CC,
	kVtbl_NiD3DRenderState = 0xA8B2AC,
	kVtbl_NiD3DGeometryGroupManager = 0xA8A778,
	kVtbl_NiD3DShaderProgram = 0xA8B818,
	kVtbl_NiD3DPixelShader = 0xA8B3CC,
	kVtbl_NiD3DVertexShader = 0xA8B41C,
	kVtbl_NiDX9TextureData = 0xA89F54,
	kVtbl_NiDX9SourceCubeMapData = 0xA89F80,
	kVtbl_NiDX9VertexBufferManager = 0xA8A5A4,
	kVtbl_NiDX9IndexBufferManager = 0xA8A770,
	kVtbl_NiDX9RenderedTextureData = 0xA88464,
	kVtbl_NiDX9RenderedCubeMapData = 0xA8A868,
	kVtbl_NiDX9DynamicTextureData = 0xA8A92C,
	kVtbl_NiDX9TextureManager = 0xA8A9DC,
	kVtbl_NiDX9RenderState = 0xA8A9F4,
	kVtbl_NiD3DShaderProgramCreator = 0xA8AB04,
	kVtbl_NiD3DShaderProgramCreatorHLSL = 0xA8AB2C,
	kVtbl_NiD3DShaderProgramCreatorAsm = 0xA8AB5C,
	kVtbl_NiD3DShaderProgramCreatorObj = 0xA8AB90,
	kVtbl_NiD3DShaderFactory = 0xA8AE6C,
	kVtbl_NiD3DShaderProgramFactory = 0xA8B2A0,
	kVtbl_NiStaticGeometryGroup = 0xA8AF5C,
	kVtbl_NiUnsharedGeometryGroup = 0xA8AF88,
	kVtbl_NiDynamicGeometryGroup = 0xA8B03C,
	kVtbl_NiD3DHLSLPixelShader = 0xA8B484,
	kVtbl_NiD3DHLSLVertexShader = 0xA8B4EC,
	kVtbl_NiDX9ShaderConstantManager = 0xA8B3BC,
	kVtbl_NiGeometryGroup = 0xA8B7E0,
	kVtbl_NiD3DShaderConstantManager = 0xA8B80C,
	kVtbl_IdvFileError = 0xA8BA5C,
	kVtbl_Random = 0xA8C5D4,
	kVtbl_CBillboardLeaf = 0xA8CAB4,
	kVtbl_MapShader = 0xA8CCDC,
	kVtbl_BlurShader = 0xA8CE1C,
	kVtbl_MenuBGShader = 0xA8CFD4,
	kVtbl_GrassLoadTask = 0xA90238,
	kVtbl_TallGrassShaderProperty = 0xA9026C,
	kVtbl_WaterShader = 0xA9170C,
	kVtbl_WaterShaderDisplacement = 0xA9193C,
	kVtbl_WaterShaderHeightMap = 0xA91B74,
	kVtbl_BSShaderProperty = 0xA91D14,
	kVtbl_ShadowLightShader = 0xA90634,
	kVtbl_ShadowSceneLight = 0xA912B4,
	kVtbl_BSRenderedTexture = 0xA91370,
	kVtbl_BSShaderPPLightingProperty = 0xA91384,
	kVtbl_ParticleShader = 0xA91E4C,
	kVtbl_ParticleShaderProperty = 0xA91FA4,
	kVtbl_TallGrassShader = 0xA9202C,
	kVtbl_BlurShader_P20 = 0xA9224C,
	kVtbl_GethitShader = 0xA92394,
	kVtbl_BSShaderLightingProperty = 0xA92524,
	kVtbl_PrecipitationShader = 0xA925F4,
	kVtbl_PrecipitationShaderProperty = 0xA9277C,
	kVtbl_SpeedTreeLeafShader = 0xA92844,
	kVtbl_STLSPData = 0xA929C0,
	kVtbl_SpeedTreeLeafShaderProperty = 0xA929CC,
	kVtbl_SpeedTreeBranchShaderProperty = 0xA92A94,
	kVtbl_STSPData = 0xA92B6C,
	kVtbl_SpeedTreeShaderPPLightingProperty = 0xA92B74,
	kVtbl_SpeedTreeShaderLightingProperty = 0xA92C2C,
	kVtbl_BoltShaderProperty = 0xA92D0C,
	kVtbl_BoltShader = 0xA92D8C,
	kVtbl_NighteyeShader = 0xA92EB4,
	kVtbl_DebugShader = 0xA92FCC,
	kVtbl_Lighting30Shader = 0xA930C4,
	kVtbl_RefractionShader = 0xA9336C,
	kVtbl_BSShader = 0xA93494,
	kVtbl_BSImageSpaceShader = 0xA9367C,
	kVtbl_CopyShader = 0xA93E14,
	kVtbl_GeometryDecalShader = 0xA93F2C,
	kVtbl_ParallaxShader = 0xA9405C,
	kVtbl_SkinShader = 0xA941AC,
	kVtbl_HairShader = 0xA942A4,
	kVtbl_SpeedTreeFrondShader = 0xA9443C,
	kVtbl_SpeedTreeBranchShader = 0xA9459C,
	kVtbl_DistantLODShader = 0xA946B4,
	kVtbl_BSCubeMapCamera = 0xA94854,
	kVtbl_WaterShaderProperty = 0xA956EC,
	kVtbl_Lighting30ShaderProperty = 0xA9576C,
	kVtbl_TallGrassTriStrips = 0xA9587C,
	kVtbl_TallGrassTriShape = 0xA9595C,
	kVtbl_GeometryDecalShaderProperty = 0xA95A1C,
	kVtbl_HairShaderProperty = 0xA95ABC,
	kVtbl_hkMemory = 0xA95C00,
	kVtbl_hkShapeCollectionFilter = 0xA95C3C,
	kVtbl_hkRayShapeCollectionFilter = 0xA95C48,
	kVtbl_ahkWorld = 0xA95C54,
	kVtbl_hkCollidableCollidableFilter = 0xA95C60,
	kVtbl_hkRayCollidableFilter = 0xA95C68,
	kVtbl_bhkMemory = 0xA95C70,
	kVtbl_bhkWorld = 0xA95CF4,
	kVtbl_hkFirstCdBodyPairCollector = 0xA9626C,
	kVtbl_bhkAvoidBox = 0xA9627C,
	kVtbl_hkAvoidBox = 0xA96324,
	kVtbl_bhkBlendCollisionObjectAddRotation = 0xA96384,
	kVtbl_bhkBlendCollisionObject = 0xA9643C,
	kVtbl_hkCharacterProxyListener = 0xA96570,
	kVtbl_bhkShapeCollection = 0xA965D4,
	kVtbl_bhkCharacterProxy = 0xA9667C,
	kVtbl_bhkListShape = 0xA96704,

};