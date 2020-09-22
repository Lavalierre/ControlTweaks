// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {
	
	template <class T>
	inline void zClamp(T& x, const T min, const T max) {
		if (x < min) x = min; else
			if (x > max) x = max;
	}



	HOOK Ivk_oCAIHuman_PC_SpecialMove		AS		(&oCAIHuman::PC_SpecialMove, &oCAIHuman::PC_SpecialMove_mod);
	HOOK Ivk_oCAIHuman_PC_Turnings			AS		(&oCAIHuman::PC_Turnings, &oCAIHuman::PC_Turnings_mod);

	HOOK Ivk_oCAniCtrl_JumpForward			AS		(&oCAniCtrl_Human::JumpForward, &oCAniCtrl_Human::JumpForward_mod);

	HOOK Ivk_oCNpcInventory_HandleEvent		AS		(&oCNpcInventory::HandleEvent, &oCNpcInventory::HandleEvent_mod);
	HOOK Ivk_oCNpcContainer_HandleEvent		AS		(&oCNpcContainer::HandleEvent, &oCNpcContainer::HandleEvent_mod);
	HOOK Ivk_oCStealContainer_HandleEvent	AS		(&oCStealContainer::HandleEvent, &oCStealContainer::HandleEvent_mod);
	HOOK Ivk_oCItemContainer_HandleEvent	AS		(&oCItemContainer::HandleEvent, &oCItemContainer::HandleEvent_mod);
	HOOK Ivk_oCViewTrade_HandleEvent		AS		(&oCViewDialogTrade::HandleEvent, &oCViewDialogTrade::HandleEvent_mod);



	zBOOL oCAIHuman::PC_SpecialMove_mod(zBOOL pressed)
	{
		if (pressed)
		{
			if (zinput->GetToggled(GAME_SMOVE))
			{
				if (GetActionMode() == ANI_ACTION_SWIM)
				{
					if (JumpForward() == 0)
					{
						GetModel()->StartAni(t_swim_2_dive, 0);
						SetWalkMode(ANI_WALKMODE_DIVE);
						SetActionMode(ANI_ACTION_DIVE);
						return TRUE;
					}
					else
					{
						return TRUE;
					}
				}
				else
				{
					return THISCALL(Ivk_oCAIHuman_PC_SpecialMove)(pressed);
				}
			}
		}

		return FALSE;
	}

	void oCAIHuman::PC_Turnings_mod(zBOOL forceRotation)
	{
		static zBOOL mouseScaleInitialized = FALSE;
		static zREAL globalMouseScale = 2.0f;

		const zREAL MOUSECLAMP = 50.0f;
		const zREAL MOUSESCALE = 10.0f;

		if (ztimer->frameTimeFloat == 0.0F) return;

		if (!mouseScaleInitialized)
		{
			mouseScaleInitialized = TRUE;
			globalMouseScale = zoptions->ReadReal("ENGINE", "zMouseRotationScale", globalMouseScale);
		}

		npc->AvoidShrink(1000);

		//changed [dennis]
		if (!Pressed(GAME_ACTION) || forceRotation)
		{
			zREAL xPos, yPos, zPos;
			zinput->GetMousePos(xPos, yPos, zPos);
			xPos *= globalMouseScale;

			if (npc->GetWeaponMode() == NPC_WEAPON_NONE)
			{
				zClamp(xPos, -MOUSECLAMP, +MOUSECLAMP);

				if (Pressed(GAME_LEFT))		PC_Turn(-1, TRUE);					else
				if (Pressed(GAME_RIGHT))	PC_Turn(+1, TRUE);					else
				if (xPos < 0)				PC_Turn((xPos / MOUSECLAMP) * MOUSESCALE, -xPos > MOUSECLAMP / MOUSESCALE);	else
				if (xPos > 0)				PC_Turn((xPos / MOUSECLAMP) * MOUSESCALE, xPos > MOUSECLAMP / MOUSESCALE);
				else
				{
					StopTurnAnis();
				}
			}
			else
			{
				zClamp(xPos, -MOUSECLAMP, +MOUSECLAMP);

				if (Pressed(GAME_LEFT))		PC_Turn(-2, TRUE);		else
				if (Pressed(GAME_RIGHT))	PC_Turn(+2, TRUE);		else
				if (xPos < 0)				PC_Turn((xPos / MOUSECLAMP) * MOUSESCALE, -xPos > MOUSECLAMP / MOUSESCALE);	else
				if (xPos > 0)				PC_Turn((xPos / MOUSECLAMP) * MOUSESCALE, xPos > MOUSECLAMP / MOUSESCALE);
				else
				{
					StopTurnAnis();
				}
			}
		}
		else StopTurnAnis();
	}






	int oCAniCtrl_Human::JumpForward_mod()
	{
		if (GetWaterLevel() == 2)
		{
			if (world->TraceRayNearestHit(player->GetPositionWorld(), player->GetAtVectorWorld() * 150, (zCVob*)Null, zTRACERAY_STAT_POLY))
			{
				zVEC3 originPos = centerPos;
				centerPos = world->traceRayReport.foundIntersection;
				CanJumpLedge();
				centerPos = originPos;
				if (GetFoundLedge()) {
					zREAL ledgeYDist = GetLedgeInfo()->point[VY] - feetY;
					if (ledgeYDist > config.zMV_YMAX_JUMPMID) {
						GetModel()->StartAni(_t_stand_2_jumpup, 0);
						npc->SetBodyState(BS_JUMP);
						return 3;
					}
					else if (ledgeYDist > config.zMV_YMAX_JUMPLOW) {
						GetModel()->StartAni(t_stand_2_jumpupmid, 0);
						npc->SetBodyState(BS_JUMP);
						return 2;
					}
					else if (ledgeYDist > config.zMV_STEP_HEIGHT) {
						GetModel()->StartAni(t_stand_2_jumpuplow, 0);
						npc->SetBodyState(BS_JUMP);
						return 1;
					}
					else {
						GetModel()->StartAni(_t_stand_2_jump, 0);
						npc->SetBodyState(BS_JUMP);
						ClearFoundLedge();
						return 4;
					}

				}
			}
		}
		else
		{
			return THISCALL(Ivk_oCAniCtrl_JumpForward)();
		}

		return 0;
	}









	zBOOL oCNpcInventory::HandleEvent_mod(int key)
	{
		if (!IsActive()) return FALSE;
		if (!owner->GetEM()->IsEmpty(TRUE)) return FALSE;
		if (owner->interactItemCurrentState != -1) return FALSE;

		zINT transferAmount = 1;

		if (!GetNextContainerLeft(this))
		{
			oCItem* item = GetSelectedItem();
			if (key == MOUSE_WHEELDOWN && zinput->GetState(GAME_SLOW))
			{
				if (item->HasFlag(ITM_FLAG_ACTIVE) && item->SplitSlot())
				{
					owner->GetEM()->OnMessage(zNEW(oCMsgManipulate)(oCMsgManipulate::EV_EQUIPITEM, item), owner);
				}
				else
				{
					bool canDropItem = true;

					for (int i = 0; i < owner->GetEM()->GetNumMessages(); i++)
					{
						oCMsgManipulate* pMsg = dynamic_cast<oCMsgManipulate*>(owner->GetEM()->GetEventMessage(i));
						if (pMsg)
						{
							if (pMsg->GetSubType() == oCMsgManipulate::EV_DROPVOB)
							{
								canDropItem = false;
								break;
							}
						}
					}

					if (canDropItem)
					{
						if (item->amount > 1)
						{
							oCItem* splitItem = (oCItem*)item->CreateCopy();
							if (splitItem)
							{
								splitItem->amount = 1;
								item = splitItem;
							}
						}

						owner->GetEM()->OnMessage(zNEW(oCMsgManipulate)(oCMsgManipulate::EV_DROPVOB, item), owner);
						return TRUE;
					}

					return FALSE;
				}
			}
		}
		else
		{
			if (key == MOUSE_WHEELDOWN && zinput->GetState(GAME_SLOW))
				key = MOUSE_BUTTONLEFT;
		}

		return THISCALL(Ivk_oCNpcInventory_HandleEvent)(key);

	}
	
	zBOOL oCNpcContainer::HandleEvent_mod(int key)
	{
		if (key == MOUSE_WHEELDOWN && zinput->GetState(GAME_SLOW))
			key = MOUSE_BUTTONLEFT;

		return THISCALL(Ivk_oCNpcContainer_HandleEvent)(key);
	}

	zBOOL oCStealContainer::HandleEvent_mod(int key)
	{
		if (key == MOUSE_WHEELDOWN && zinput->GetState(GAME_SLOW))
			key = MOUSE_BUTTONLEFT;

		return THISCALL(Ivk_oCStealContainer_HandleEvent)(key);
	}

	zBOOL oCItemContainer::HandleEvent_mod(int key)
	{
		if (key == MOUSE_WHEELDOWN && zinput->GetState(GAME_SLOW))
			key = MOUSE_BUTTONLEFT;

		return THISCALL(Ivk_oCItemContainer_HandleEvent)(key);
	}

	zBOOL oCViewDialogTrade::HandleEvent_mod(int key)
	{
		if (key == MOUSE_WHEELDOWN && zinput->GetState(GAME_SLOW))
			key = MOUSE_BUTTONLEFT;

		return THISCALL(Ivk_oCViewTrade_HandleEvent)(key);
	}
}