#include "CrashLogger.hpp"
#include "Formatter.hpp"
#include "RTTI.hpp"
#include "GameForms.h"

namespace CrashLogger::Labels
{
	void FillOBSELabels()
	{
		/*
		Push(0x10F1EE0, nullptr, "TypeInfo");
		Push(0x10B9D28, nullptr, "TileShaderProperty");

		Push<LabelGlobal>(0x11F3374, AsUInt32, "TileValueIndirectTemp");
		Push<LabelGlobal>(0x118FB0C, nullptr, "ShowWhoDetects");
		Push<LabelGlobal>(0x1042C58, nullptr, "ShowWhoDetects");
		Push<LabelGlobal>(0x011F6238, nullptr, "MemoryManager");

		Push<LabelEmpty>(0x1000000, nullptr); // integer that is often encountered
		Push<LabelEmpty>(0x11C0000, nullptr);
		*/

		Push(kVtbl_MemoryHeap);

		Push(kVtbl_Menu);
		Push(kVtbl_StatsMenu);
		Push(kVtbl_TextEditMenu);
		Push(kVtbl_QuantityMenu);
		Push(kVtbl_BookMenu);
		Push(kVtbl_ContainerMenu);
		Push(kVtbl_CreditsMenu);
		Push(kVtbl_DialogMenu);
		Push(kVtbl_HUDMainMenu);
		Push(kVtbl_InventoryMenu);
		Push(kVtbl_LevelUpMenu);
		Push(kVtbl_LoadingMenu);
		Push(kVtbl_LockpickMenu);
		Push(kVtbl_MapMenu);
		Push(kVtbl_MessageMenu);
		Push(kVtbl_RaceSexMenu);
		Push(kVtbl_RepairMenu);
		Push(kVtbl_SleepWaitMenu);

		Push(kVtbl_Tile);
		Push(kVtbl_TileMenu);
		Push(kVtbl_TileRect);
		Push(kVtbl_TileImage);
		Push(kVtbl_GameSettingCollection, nullptr);
		Push(kVtbl_INISettingCollection);

		Push(kVtbl_TESForm, As<TESForm>);
		Push(kVtbl_AlchemyItem, As<TESForm>);
		Push(kVtbl_EffectSetting, As<TESForm>);
		Push(kVtbl_EnchantmentItem, As<TESForm>);
		Push(kVtbl_IngredientItem, As<TESForm>);
		Push(kVtbl_MagicItemForm, As<TESForm>);
		Push(kVtbl_MagicItemObject, As<TESForm>);
		Push(kVtbl_Script, As<TESForm>);
		Push(kVtbl_SpellItem, As<TESForm>);
		Push(kVtbl_TESActorBase, As<TESForm>);
		Push(kVtbl_TESAmmo, As<TESForm>);
		Push(kVtbl_TESBoundAnimObject, As<TESForm>);
		Push(kVtbl_TESBoundObject, As<TESForm>);
		Push(kVtbl_TESBoundTreeObject, As<TESForm>);
		Push(kVtbl_TESClass, As<TESForm>);
		Push(kVtbl_TESClimate, As<TESForm>);
		Push(kVtbl_TESCombatStyle, As<TESForm>);
		Push(kVtbl_TESCreature, As<TESForm>);
		Push(kVtbl_TESEffectShader, As<TESForm>);
		Push(kVtbl_TESEyes, As<TESForm>);
		Push(kVtbl_TESFaction, As<TESForm>);
		Push(kVtbl_TESFlora, As<TESForm>);
		Push(kVtbl_TESFurniture, As<TESForm>);
		Push(kVtbl_TESGlobal, As<TESForm>);
		Push(kVtbl_TESGrass, As<TESForm>);
		Push(kVtbl_TESHair, As<TESForm>);
		Push(kVtbl_TESIdleForm, As<TESForm>);
		Push(kVtbl_TESKey, As<TESForm>);
		Push(kVtbl_TESLandTexture, As<TESForm>);
		Push(kVtbl_TESLevCreature, As<TESForm>);
		Push(kVtbl_TESLevItem, As<TESForm>);
		Push(kVtbl_TESLevSpell, As<TESForm>);
		Push(kVtbl_TESLoadScreen, As<TESForm>);
		Push(kVtbl_TESNPC, As<TESForm>);
		Push(kVtbl_TESObject, As<TESForm>);
		Push(kVtbl_TESObjectACTI, As<TESForm>);
		Push(kVtbl_TESObjectANIO, As<TESForm>);
		Push(kVtbl_TESObjectARMO, As<TESForm>);
		Push(kVtbl_TESObjectBOOK, As<TESForm>);
		Push(kVtbl_TESObjectCELL, As<TESForm>);
		Push(kVtbl_TESObjectCLOT, As<TESForm>);
		Push(kVtbl_TESObjectCONT, As<TESForm>);
		Push(kVtbl_TESObjectDOOR, As<TESForm>);
		Push(kVtbl_TESObjectLIGH, As<TESForm>);
		Push(kVtbl_TESObjectMISC, As<TESForm>);
		Push(kVtbl_TESObjectSTAT, As<TESForm>);
		Push(kVtbl_TESObjectTREE, As<TESForm>);
		Push(kVtbl_TESObjectWEAP, As<TESForm>);
		Push(kVtbl_TESPathgrid, As<TESPathGrid>);
		Push(kVtbl_TESQuest, As<TESForm>);
		Push(kVtbl_TESRace, As<TESForm>);
		Push(kVtbl_TESRegion, As<TESForm>);
		Push(kVtbl_TESSkill, As<TESForm>);
		Push(kVtbl_TESSound, As<TESForm>);
		Push(kVtbl_TESTopic, As<TESForm>);
		Push(kVtbl_TESTopicInfo, As<TESForm>);
		Push(kVtbl_TESWaterForm, As<TESForm>);
		Push(kVtbl_TESWeather, As<TESForm>);
		Push(kVtbl_TESWorldSpace, As<TESForm>);

		Push(kVtbl_TESPackage, As<TESPackage>);
		Push(kVtbl_AlarmPackage);
		Push(kVtbl_DialoguePackage);
		Push(kVtbl_FleePackage);
		Push(kVtbl_SpectatorPackage);
		Push(kVtbl_TrespassPackage);

		Push(kVtbl_TESObjectREFR, As<TESObjectREFR>);
		Push(kVtbl_MobileObject);
		Push(kVtbl_Actor, As<TESForm>);
		Push(kVtbl_Creature, As<TESObjectREFR>);
		Push(kVtbl_Character, As<TESObjectREFR>);
		Push(kVtbl_PlayerCharacter);
		Push(kVtbl_ArrowProjectile);

		Push(kVtbl_QueuedReference, As<QueuedReference>);
		Push(kVtbl_QueuedCharacter, As<QueuedReference>);
		Push(kVtbl_QueuedCreature, As<QueuedReference>);
		Push(kVtbl_QueuedPlayer);

		Push(kVtbl_BaseProcess);
		Push(kVtbl_LowProcess);
		Push(kVtbl_MiddleLowProcess);
		Push(kVtbl_MiddleHighProcess);
		Push(kVtbl_HighProcess);



		Push(kVtbl_TESTexture, As<TESTexture>);
		Push(kVtbl_TESIcon);
		Push(kVtbl_QueuedTexture);


		// Ni
		Push(kVtbl_NiNode, As<NiNode>);
		Push(kVtbl_NiObjectNET, As<NiObjectNET>);
		Push(kVtbl_NiExtraData, As<NiExtraData>);
		Push(kVtbl_NiTriStrips, As<NiTriStrips>);
		// NiProperty
		Push(kVtbl_NiVertexColorProperty);
		//		Push(kVtbl_NiWireframeProperty);
		Push(kVtbl_NiZBufferProperty);
		Push(kVtbl_NiAlphaProperty);
		Push(kVtbl_NiStencilProperty);
		//		Push(kVtbl_NiRendererSpecificProperty);

		Push(kVtbl_SkyShaderProperty);
		Push(kVtbl_DistantLODShaderProperty);
		Push(kVtbl_TallGrassShaderProperty);
		//		Push(kVtbl_SpeedTreeFrondShaderProperty);
		Push(kVtbl_BSFogProperty);

		// unk
		Push(kVtbl_ExtraRefractionProperty);

		// NiTexture
		//		Push(kVtbl_NiDX9Direct3DTexture);

		// NiAVObject
		Push(kVtbl_NiPointLight);

		Push(kVtbl_BSTreeNode);
		Push(kVtbl_SceneGraph);
		Push(kVtbl_BSTempNode);
		Push(kVtbl_BSTempNodeManager);
		Push(kVtbl_BSClearZNode);
		Push(kVtbl_BSFadeNode); // missing RTTI name
		Push(kVtbl_BSFaceGenNiNode);
		Push(kVtbl_NiBillboardNode);
		Push(kVtbl_ShadowSceneNode); // 10ADCE0

		// NiGeometry
		Push(kVtbl_BSScissorTriShape);
		Push(kVtbl_NiParticleSystem);
		Push(kVtbl_NiMeshParticleSystem);

		// NiRenderer
		Push(kVtbl_NiDX9Renderer);

		// Misc
		Push(kVtbl_bhkCollisionObject);
		Push(kVtbl_WeaponObject);
		Push(kVtbl_bhkPCollisionObject);
		Push(kVtbl_bhkSPCollisionObject);

		// Animations

		// NiTimeController
		//		Push(kVtbl_BSDoorHavokController);
		Push(kVtbl_BSPlayerDistanceCheckController);
		//		Push(kVtbl_NiD3DController);
		Push(kVtbl_NiPSysModifierCtlr);
		Push(kVtbl_NiPSysEmitterCtlr);
		Push(kVtbl_NiPSysModifierBoolCtlr);
		Push(kVtbl_NiPSysModifierActiveCtlr);
		Push(kVtbl_NiPSysModifierFloatCtlr);
		Push(kVtbl_NiPSysInitialRotSpeedVarCtlr);
		Push(kVtbl_NiPSysInitialRotSpeedCtlr);
		Push(kVtbl_NiPSysInitialRotAngleVarCtlr);
		Push(kVtbl_NiPSysInitialRotAngleCtlr);
		Push(kVtbl_NiPSysGravityStrengthCtlr);
		Push(kVtbl_NiPSysFieldMaxDistanceCtlr);
		Push(kVtbl_NiPSysFieldMagnitudeCtlr);
		Push(kVtbl_NiPSysFieldAttenuationCtlr);
		Push(kVtbl_NiPSysEmitterSpeedCtlr);
		Push(kVtbl_NiPSysEmitterPlanarAngleVarCtlr);
		Push(kVtbl_NiPSysEmitterPlanarAngleCtlr);
		Push(kVtbl_NiPSysEmitterLifeSpanCtlr);
		Push(kVtbl_NiPSysEmitterInitialRadiusCtlr);
		Push(kVtbl_NiPSysEmitterDeclinationVarCtlr);
		Push(kVtbl_NiPSysEmitterDeclinationCtlr);
		Push(kVtbl_NiPSysAirFieldSpreadCtlr);
		Push(kVtbl_NiPSysAirFieldInheritVelocityCtlr);
		Push(kVtbl_NiPSysAirFieldAirFrictionCtlr);

		Push(kVtbl_NiPSysUpdateCtlr);
		Push(kVtbl_NiPSysResetOnLoopCtlr);

		//		Push(kVtbl_bhkAvoidBox);
		Push(kVtbl_bhkSerializable);
		Push(kVtbl_bhkEntity);
		Push(kVtbl_bhkRigidBody);
		Push(kVtbl_bhkLimitedHingeConstraint);
		Push(kVtbl_bhkAction);
		Push(kVtbl_bhkUnaryAction);
		Push(kVtbl_bhkWorldObject);
		Push(kVtbl_bhkPhantom);
		Push(kVtbl_bhkShapePhantom);
		Push(kVtbl_bhkSimpleShapePhantom);
		//		Push(kVtbl_bhkPointToPathConstraint);
		//		Push(kVtbl_bhkPoweredHingeConstraint);

		Push(kVtbl_bhkShape);
		Push(kVtbl_bhkTransformShape);
		Push(kVtbl_bhkSphereRepShape);
		Push(kVtbl_bhkConvexShape);
		Push(kVtbl_bhkSphereShape);
		Push(kVtbl_bhkCapsuleShape);
		Push(kVtbl_bhkMultiSphereShape);
		Push(kVtbl_bhkBvTreeShape);
		Push(kVtbl_bhkTriSampledHeightFieldBvTreeShape);
		Push(kVtbl_bhkCharacterListenerArrow);
		Push(kVtbl_bhkCharacterListenerSpell);

		//		Push(kVtbl_TESObjectExtraData);
		Push(kVtbl_BSFaceGenAnimationData);
		Push(kVtbl_BSFaceGenModelExtraData);
		Push(kVtbl_BSFaceGenBaseMorphExtraData);
		Push(kVtbl_DebugTextExtraData);
		Push(kVtbl_NiFloatExtraData);
		//		Push(kVtbl_FadeNodeMaxAlphaExtraData);
		Push(kVtbl_NiBinaryExtraData);

		// Geometry data
		Push(kVtbl_NiAdditionalGeometryData, nullptr);
		Push(kVtbl_BSPackedAdditionalGeometryData);
		Push(kVtbl_NiPSysData);
		Push(kVtbl_NiMeshPSysData);

		Push(kVtbl_NiTask, nullptr);
		//		Push(kVtbl_BSTECreateTask);
		//		Push(kVtbl_NiParallelUpdateTaskManager::SignalTask);
		Push(kVtbl_NiPSysUpdateTask);
		Push(kVtbl_NiSkinInstance);
		Push(kVtbl_NiPSysModifier);
		Push(kVtbl_NiPSysMeshUpdateModifier);
		Push(kVtbl_NiPSysRotationModifier);
		Push(kVtbl_NiPSysEmitter);
		Push(kVtbl_NiPSysMeshEmitter);
		Push(kVtbl_NiPSysVolumeEmitter);
		Push(kVtbl_NiPSysCylinderEmitter);
		Push(kVtbl_NiPSysSphereEmitter);
		Push(kVtbl_NiPSysBoxEmitter);
		Push(kVtbl_NiPSysGravityModifier);
		Push(kVtbl_NiPSysSpawnModifier);
		Push(kVtbl_NiPSysPositionModifier);
		Push(kVtbl_NiPSysGrowFadeModifier);
		Push(kVtbl_NiPSysDragModifier);
		Push(kVtbl_NiPSysColorModifier);
		Push(kVtbl_NiPSysColliderManager);
		Push(kVtbl_NiPSysBoundUpdateModifier);
		Push(kVtbl_NiPSysBombModifier);
		Push(kVtbl_NiPSysAgeDeathModifier);
		Push(kVtbl_NiPSysVortexFieldModifier);
		Push(kVtbl_NiPSysTurbulenceFieldModifier);
		Push(kVtbl_NiPSysRadialFieldModifier);
		Push(kVtbl_NiPSysGravityFieldModifier);
		Push(kVtbl_NiPSysDragFieldModifier);
		Push(kVtbl_NiPSysAirFieldModifier);
		Push(kVtbl_NiPSysEmitterCtlrData);
		//		Push(kVtbl_BSShaderAccumulator);
		Push(kVtbl_NiPSysCollider);
		Push(kVtbl_NiPSysSphericalCollider);
		Push(kVtbl_NiPSysPlanarCollider);


		// NiStream
		Push(kVtbl_BSStream);


		// animations
		Push(kVtbl_BSAnimGroupSequence);
		Push(kVtbl_AnimSequenceBase);
		Push(kVtbl_AnimSequenceSingle);
		Push(kVtbl_AnimSequenceMultiple);

		Push(kVtbl_QueuedKF, As<QueuedKF>);
		Push(kVtbl_QueuedAnimIdle);

		Push(kVtbl_BSFile), As<BSFile>;
		Push(kVtbl_ArchiveFile);
		Push(kVtbl_CompressedArchiveFile);

		// model
		Push(kVtbl_TESModel), As<TESModel>;
		Push(kVtbl_QueuedModel, As<QueuedModel>);

		// effects
		Push(kVtbl_ScriptEffect, As<ScriptEffect>);
		Push(kVtbl_ActiveEffect, As<ActiveEffect>);
		Push(kVtbl_AbsorbEffect);
		Push(kVtbl_AssociatedItemEffect);
		Push(kVtbl_BoundItemEffect);
		Push(kVtbl_CalmEffect);
		Push(kVtbl_ChameleonEffect);
		Push(kVtbl_CommandCreatureEffect);
		Push(kVtbl_CommandEffect);
		Push(kVtbl_CommandHumanoidEffect);
		Push(kVtbl_CureEffect);
		Push(kVtbl_DarknessEffect);
		Push(kVtbl_DemoralizeEffect);
		Push(kVtbl_DetectLifeEffect);
		Push(kVtbl_DisintegrateArmorEffect);
		Push(kVtbl_DisintegrateWeaponEffect);
		Push(kVtbl_DispelEffect);
		Push(kVtbl_FrenzyEffect);
		Push(kVtbl_InvisibilityEffect);
		Push(kVtbl_LightEffect);
		Push(kVtbl_LockEffect);
		Push(kVtbl_MagicHitEffect);
		Push(kVtbl_MagicModelHitEffect);
		Push(kVtbl_MagicShaderHitEffect);
		Push(kVtbl_NightEyeEffect);
		Push(kVtbl_OpenEffect);
		Push(kVtbl_ParalysisEffect);
		Push(kVtbl_ReanimateEffect);
		Push(kVtbl_ShieldEffect);
		Push(kVtbl_SummonCreatureEffect);
		Push(kVtbl_SunDamageEffect);
		Push(kVtbl_TelekinesisEffect);
		Push(kVtbl_TurnUndeadEffect);
		Push(kVtbl_ValueModifierEffect);
		Push(kVtbl_VampirismEffect);

		Push(kVtbl_MagicItem, nullptr);

		// w
		Push(kVtbl_Archive);
		Push(kVtbl_Atmosphere);
		Push(kVtbl_AttachDistant3DTask);


		Push(kVtbl_BSCullingProcess);
		Push(kVtbl_BSExtraData);
		Push(kVtbl_BSFaceGenImage);
		Push(kVtbl_BSFaceGenKeyframe);
		Push(kVtbl_BSFaceGenKeyframeMultiple);
		Push(kVtbl_BSFaceGenModel);
		Push(kVtbl_BSFaceGenModelMap);
		Push(kVtbl_BSFaceGenMorph);
		Push(kVtbl_BSFaceGenMorphDataHair);
		Push(kVtbl_BSFaceGenMorphDataHead);
		Push(kVtbl_BSFaceGenMorphStatistical);
		Push(kVtbl_BSSearchPath);
		Push(kVtbl_BSTaskThread);
		Push(kVtbl_BSTempEffect);
		Push(kVtbl_BSTempEffectGeometryDecal);
		Push(kVtbl_BSTempEffectParticle);
		Push(kVtbl_BSThread);
		Push(kVtbl_BSTreeModel);
		Push(kVtbl_BackgroundCloneThread);
		Push(kVtbl_BaseExtraList);
		Push(kVtbl_BlendSettingCollection);

		Push(kVtbl_CBillboardLeaf);
		Push(kVtbl_CIdvCamera);
		Push(kVtbl_CTreeEngine);
		Push(kVtbl_CellMopp);
		Push(kVtbl_ChangesMap);
		Push(kVtbl_Clouds);
		Push(kVtbl_CombatController);

		Push(kVtbl_EffectItemList);
		Push(kVtbl_ExteriorCellLoaderTask);
		Push(kVtbl_ExteriorCellNewReferencesMap);

		Push(kVtbl_ExtraAction);
		Push(kVtbl_ExtraAnim);
		Push(kVtbl_ExtraCannotWear);
		Push(kVtbl_ExtraCell3D);
		Push(kVtbl_ExtraCellCanopyShadowMask);
		Push(kVtbl_ExtraCellClimate);
		Push(kVtbl_ExtraCellMusicType);
		Push(kVtbl_ExtraCellWaterType);
		Push(kVtbl_ExtraCharge);
		Push(kVtbl_ExtraContainerChanges);
		Push(kVtbl_ExtraCount);
		Push(kVtbl_ExtraDataList);
		Push(kVtbl_ExtraDetachTime);
		Push(kVtbl_ExtraDistantData);
		Push(kVtbl_ExtraDroppedItemList);
		Push(kVtbl_ExtraEditorID);
		Push(kVtbl_ExtraEditorRefMoveData);
		Push(kVtbl_ExtraEnableStateChildren);
		Push(kVtbl_ExtraEnableStateParent);
		Push(kVtbl_ExtraFollower);
		Push(kVtbl_ExtraGhost);
		Push(kVtbl_ExtraGlobal);
		Push(kVtbl_ExtraHasNoRumors);
		Push(kVtbl_ExtraHavok);
		Push(kVtbl_ExtraHeadingTarget);
		Push(kVtbl_ExtraHealth);
		Push(kVtbl_ExtraHotkey);
		Push(kVtbl_ExtraInfoGeneralTopic);
		Push(kVtbl_ExtraItemDropper);
		Push(kVtbl_ExtraLastFinishedSequence);
		Push(kVtbl_ExtraLevCreaModifier);
		Push(kVtbl_ExtraLeveledCreature);
		Push(kVtbl_ExtraLeveledItem);
		Push(kVtbl_ExtraLight);
		Push(kVtbl_ExtraMapMarker);
		Push(kVtbl_ExtraMerchantContainer);
		Push(kVtbl_ExtraNorthRotation);
		Push(kVtbl_ExtraOriginalReference);
		Push(kVtbl_ExtraOwnership);
		Push(kVtbl_ExtraPackage);
		Push(kVtbl_ExtraPackageStartLocation);
		Push(kVtbl_ExtraPersistentCell);
		Push(kVtbl_ExtraPoison);
		Push(kVtbl_ExtraProcessMiddleLow);
		Push(kVtbl_ExtraRagDollData);
		Push(kVtbl_ExtraRandomTeleportMarker);
		Push(kVtbl_ExtraRank);
		Push(kVtbl_ExtraReferencePointer);


		Push(kVtbl_ExtraRegionList);
		Push(kVtbl_ExtraRunOncePacks);
		Push(kVtbl_ExtraScale);
		Push(kVtbl_ExtraScript);
		Push(kVtbl_ExtraSeed);
		Push(kVtbl_ExtraSeenData);
		Push(kVtbl_ExtraSound);
		Push(kVtbl_ExtraStartingPosition);
		Push(kVtbl_ExtraStartingWorldOrCell);
		Push(kVtbl_ExtraTeleport);
		Push(kVtbl_ExtraTimeLeft);
		Push(kVtbl_ExtraTresPassPackage);
		Push(kVtbl_ExtraUsedMarkers);
		Push(kVtbl_ExtraUses);
		Push(kVtbl_ExtraWorn);
		Push(kVtbl_ExtraWornLeft);
		Push(kVtbl_ExtraXTarget);

		Push(kVtbl_GridArray);
		Push(kVtbl_GridCellArray);
		Push(kVtbl_HavokError);
		Push(kVtbl_HavokFileStreambufReader);
		Push(kVtbl_HavokStreambufFactory);

		Push(kVtbl_IOManager);
		Push(kVtbl_IOTask);
		Push(kVtbl_IntSeenData);
		Push(kVtbl_InteriorCellNewReferencesMap);
		Push(kVtbl_LipTask);
		Push(kVtbl_MagicBallProjectile);
		Push(kVtbl_MagicBoltProjectile);
		Push(kVtbl_MagicCaster);
		Push(kVtbl_MagicFogProjectile);
		Push(kVtbl_MagicProjectile);
		Push(kVtbl_MagicSprayProjectile);
		Push(kVtbl_MagicTarget);
		Push(kVtbl_MessageHandler);
		Push(kVtbl_Moon);

		Push(kVtbl_NiBinaryStream);
		Push(kVtbl_NiD3DDefaultShader);
		Push(kVtbl_NiD3DGeometryGroupManager);
		Push(kVtbl_NiD3DHLSLPixelShader);
		Push(kVtbl_NiD3DHLSLVertexShader);
		Push(kVtbl_NiD3DPass);
		Push(kVtbl_NiD3DPixelShader);
		Push(kVtbl_NiD3DRenderState);
		Push(kVtbl_NiD3DShader);
		Push(kVtbl_NiD3DShaderConstantManager);
		Push(kVtbl_NiD3DShaderDeclaration);
		Push(kVtbl_NiD3DShaderFactory);
		Push(kVtbl_NiD3DShaderInterface);
		Push(kVtbl_NiD3DShaderProgram);
		Push(kVtbl_NiD3DShaderProgramCreator);
		Push(kVtbl_NiD3DShaderProgramCreatorAsm);
		Push(kVtbl_NiD3DShaderProgramCreatorHLSL);
		Push(kVtbl_NiD3DShaderProgramCreatorObj);
		Push(kVtbl_NiD3DShaderProgramFactory);
		Push(kVtbl_NiD3DVertexShader);
		Push(kVtbl_NiDX92DBufferData);
		Push(kVtbl_NiDX9AdditionalDepthStencilBufferData);
		Push(kVtbl_NiDX9DynamicTextureData);
		Push(kVtbl_NiDX9ImplicitBufferData);
		Push(kVtbl_NiDX9ImplicitDepthStencilBufferData);
		Push(kVtbl_NiDX9IndexBufferManager);
		Push(kVtbl_NiDX9RenderState);
		Push(kVtbl_NiDX9RenderedCubeMapData);
		Push(kVtbl_NiDX9RenderedTextureData);
		Push(kVtbl_NiDX9ShaderConstantManager);
		Push(kVtbl_NiDX9ShaderDeclaration);
		Push(kVtbl_NiDX9SourceCubeMapData);
		Push(kVtbl_NiDX9SourceTextureData);
		Push(kVtbl_NiDX9TextureBufferData);
		Push(kVtbl_NiDX9TextureData);
		Push(kVtbl_NiDX9TextureManager);
		Push(kVtbl_NiDX9VertexBufferManager);

		Push(kVtbl_NiDynamicGeometryGroup);
		Push(kVtbl_NiFile);

		Push(kVtbl_NiGeometryGroup);

		Push(kVtbl_NiMemStream);
		Push(kVtbl_NiPSysFieldModifier);
		Push(kVtbl_NiRefObject);
		Push(kVtbl_NiSearchPath);

		Push(kVtbl_NiStaticGeometryGroup);

		Push(kVtbl_NiUnsharedGeometryGroup);

		Push(kVtbl_NonActorMagicCaster);
		Push(kVtbl_NonActorMagicTarget);
		Push(kVtbl_Normal);
		Push(kVtbl_NumericIDBufferMap);

		Push(kVtbl_PosGen);
		Push(kVtbl_Precipitation);

		Push(kVtbl_QueuedChildren);
		Push(kVtbl_QueuedFile);
		Push(kVtbl_QueuedFileEntry);
		Push(kVtbl_QueuedHead);
		Push(kVtbl_QueuedHelmet);
		Push(kVtbl_QueuedMagicItem);
		Push(kVtbl_QueuedTree);
		Push(kVtbl_QueuedTreeBillboard);
		Push(kVtbl_QueuedTreeModel);

		Push(kVtbl_Random);
		Push(kVtbl_RegSettingCollection);
		Push(kVtbl_SeenData);
		Push(kVtbl_Sky);
		Push(kVtbl_SkyObject);
		Push(kVtbl_SkyTask);
		Push(kVtbl_SpecificItemCollector);
		Push(kVtbl_Stars);
		Push(kVtbl_Sun);
		Push(kVtbl_TES);
		Push(kVtbl_TESAIForm);
		Push(kVtbl_TESActorBaseData);
		Push(kVtbl_TESAnimGroup);
		Push(kVtbl_TESAnimation);
		Push(kVtbl_TESArrowTargetListener);
		Push(kVtbl_TESAttackDamageForm);
		Push(kVtbl_TESAttributes);
		Push(kVtbl_TESBipedModelForm);
		Push(kVtbl_TESChildCell);
		Push(kVtbl_TESContainer);
		Push(kVtbl_TESDescription);
		Push(kVtbl_TESEnchantableForm);
		Push(kVtbl_TESFullName);
		Push(kVtbl_TESHealthForm);
		Push(kVtbl_TESIconTree);
		Push(kVtbl_TESLeveledList);
		Push(kVtbl_TESModelAnim);
		Push(kVtbl_TESModelList);
		Push(kVtbl_TESModelTree);
		Push(kVtbl_TESObjectLAND);
		Push(kVtbl_TESProduceForm);
		Push(kVtbl_TESRaceForm);
		Push(kVtbl_TESReactionForm);
		Push(kVtbl_TESRegionData);
		Push(kVtbl_TESRegionDataGrass);
		Push(kVtbl_TESRegionDataLandscape);
		Push(kVtbl_TESRegionDataManager);
		Push(kVtbl_TESRegionDataMap);
		Push(kVtbl_TESRegionDataSound);
		Push(kVtbl_TESRegionDataWeather);
		Push(kVtbl_TESRegionGrassObject);
		Push(kVtbl_TESRegionGrassObjectList);
		Push(kVtbl_TESRegionList);
		Push(kVtbl_TESScriptableForm);
		Push(kVtbl_TESSoundFile);
		Push(kVtbl_TESSpellList);
		Push(kVtbl_TESTrapListener);
		Push(kVtbl_TESValueForm);
		Push(kVtbl_TESWaterListener);
		Push(kVtbl_TESWeightForm);
		Push(kVtbl_TESWindListener);
		Push(kVtbl_TopicInfoArray);
		Push(kVtbl_Uniform);

		Push(kVtbl_bhkCharacterListener);
		Push(kVtbl_bhkEntityListener);
		Push(kVtbl_bhkTelekinesisListener);

		Push(kVtbl_hkBaseObject);
		Push(kVtbl_hkConstraintCinfo);
		Push(kVtbl_hkLimitedHingeConstraintCinfo);
		Push(kVtbl_hkReferencedObject);

		Push(kVtbl_hkpAllCdBodyPairCollector);
		Push(kVtbl_hkpAllCdPointCollector);
		Push(kVtbl_hkpAllRayHitCollector);
		Push(kVtbl_hkpBoxShape);
		Push(kVtbl_hkpBroadPhaseCastCollector);
		Push(kVtbl_hkpCdBodyPairCollector);
		Push(kVtbl_hkpCdPointCollector);
		Push(kVtbl_hkClosestRayHitCollector);
		Push(kVtbl_hkpEntityActivationListener);
		Push(kVtbl_hkpEntityListener);
		Push(kVtbl_hkpPhantomListener);
		Push(kVtbl_hkpRayHitCollector);

		Push(kVtbl_bhkWorldObject);
		Push(kVtbl_hkpAabbPhantom);

		Push(kVtbl_GrassLoadTask);
		Push(kVtbl_WaterShader);
		Push(kVtbl_BSShaderProperty);
		Push(kVtbl_ShadowSceneLight);
		Push(kVtbl_BSRenderedTexture);
		Push(kVtbl_ParticleShader);
		Push(kVtbl_TallGrassShader);
		Push(kVtbl_BSShaderLightingProperty);
		Push(kVtbl_PrecipitationShader);
		Push(kVtbl_BSShader);
		Push(kVtbl_BSImageSpaceShader);
		Push(kVtbl_GeometryDecalShader);
		Push(kVtbl_SkinShader);
		Push(kVtbl_HairShader);
		Push(kVtbl_DistantLODShader);
		Push(kVtbl_BSCubeMapCamera);
		Push(kVtbl_WaterShaderProperty);
		Push(kVtbl_TallGrassTriStrips);
		Push(kVtbl_TallGrassTriShape);
		Push(kVtbl_GeometryDecalShaderProperty);
		Push(kVtbl_HairShaderProperty);
		Push(kVtbl_hkMemory);
		Push(kVtbl_hkShapeCollectionFilter);
		Push(kVtbl_hkRayShapeCollectionFilter);
		Push(kVtbl_ahkWorld);
		Push(kVtbl_hkCollidableCollidableFilter);
		Push(kVtbl_hkRayCollidableFilter);
		Push(kVtbl_bhkMemory);
		Push(kVtbl_bhkWorld);
		Push(kVtbl_hkFirstCdBodyPairCollector);
		Push(kVtbl_bhkAvoidBox);
		Push(kVtbl_hkAvoidBox);
		Push(kVtbl_bhkBlendCollisionObject);
		Push(kVtbl_bhkCharacterListener);
		Push(kVtbl_bhkShapeCollection);
		Push(kVtbl_bhkCharacterProxy);
		Push(kVtbl_bhkListShape);

	};
}