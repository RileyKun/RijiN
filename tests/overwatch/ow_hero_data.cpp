#include "link.h"

hero_data g_hero_data[HERO_MAX] = {
	{
		name: "Undefined",
	},
	{
		name: "DVA",
		
		value: 1500,
		
		color: 0xed93c7,
		
		view_height: 1.74f,
		
		shift_id: 0x5,
		e_id: 0x9,
		m2_id: 0xa,
		
		max_health: 300,
		max_armor: 300,
		
		attack_numshots: 11,
		attack_damage: 2.0f,
		attack_splash_min: 0.0f,
		attack_splash_max: 0.0f,
		attack_splash_radius: 0.0f,
		attack_range: 0.0f,
		attack_speed: 0.0f,
		attack_spread: 4.15f,
		attack_recovery: 0.15f,
		attack_falloff_start: 10.0f,
		attack_falloff_end: 20.0f,
		attack_headshot: 2.0f,
		attack_move_mult: -40.0f, // percentage
		
		attack2_cooldown: 1.0f,
		attack2_value: 0.0f,
		attack2_numshots: 0,
		attack2_damage: 0.0f,
		attack2_splash_min: 0.0f,
		attack2_splash_max: 0.0f,
		attack2_splash_radius: 0.0f,
		attack2_range: 10.0f,
		attack2_speed: 0.0f,
		attack2_spread: 0.0f,
		attack2_recovery: 0.0f,
		attack2_falloff_start: 0.0f,
		attack2_falloff_end: 0.0f,
		attack2_headshot: 0.0f,
		attack2_move_mult: 0.0f,
		
		ability1_cooldown: 4.0f,
		ability1_value: 0.0f,
		ability1_numshots: 0,
		ability1_damage: 10.0f,
		ability1_splash_min: 0.0f,
		ability1_splash_max: 0.0f,
		ability1_splash_radius: 0.0f,
		ability1_range: 24.0f,
		ability1_speed: 0.0f,
		ability1_headshot: 0.0f,
		ability1_move_mult: 118.0f,
		
		ability2_cooldown: 8.0f,
		ability2_value: 0.0f,
		ability2_numshots: 18,
		ability2_damage: 7.0f,
		ability2_splash_min: 1.0f,
		ability2_splash_max: 4.0f,
		ability2_splash_radius: 1.5f,
		ability2_range: 0.0f,
		ability2_speed: 40.0f,
		ability2_headshot: 0.0f,
		ability2_move_mult: 0.0f,
		
		ult_value: 0.0f,
		ult_numshots: 0,
		ult_damage: 0.0f,
		ult_splash_min: 100.0f,
		ult_splash_max: 1000.0f,
		ult_splash_radius: 20.0f,
		ult_range: 20.0f,
		ult_speed: 0.0f,
		ult_headshot: 0.0f,
	},
	{
		name: "Orisa",
		
		value: 500,
		
		color: 0x468c43,
		
		shift_id: 0x6,
		e_id: 0x4,
		//m2_id: 0x5
		
		max_health: 200.0f,
		max_armor: 250.0f,
		
		attack_damage: 11.0f,
		attack_speed: 120.0f,
		attack_spread: 1.2f,
		attack_recovery: 0.083f,
		attack_headshot: 2.0f,
		attack_move_mult: -30.0f,
		
		attack2_cooldown: 6.0f,
		attack2_splash_radius: 5.0f,
		attack2_speed: 25.0f,
		
		ability1_cooldown: 10.0f,
		
		ability2_cooldown: 10.0f,
		ability2_speed: 25.0f,
		
		ult_health: 200.0f,
		ult_damage: 50.0f, // percentage
		ult_range: 25.0f,
	},
	{
		name: "Reinhardt",
		
		value: 1000,
		
		color: 0x929da3,
		
		shift_id: 0x2,
		e_id: 0x7,
		
		max_health: 300,
		max_armor: 200,
		
		attack_damage: 85.0f,
		attack_range: 5.0f,
		attack_recovery: 0.96f,
		
		attack2_cooldown: 5.0f,
		attack2_move_mult: -30.0f,
		
		ability1_cooldown: 10.0f,
		ability1_damage: 300.0f,
		ability1_range: 49.5f,
		
		ability2_cooldown: 6.0f,
		ability2_damage: 100.0f,
		
		ult_damage: 50.0f,
		ult_range: 20.0f,
	},
	{
		name: "Roadhog",
		
		value: 1000,
		
		color: 0xb68c52,
		
		view_height: 1.60f,
		
		shift_id: 0x3,
		e_id: 0x7,
		
		max_health: 600.0f,
		
		attack_numshots: 25,
		attack_damage: 6.6f,
		//attack_speed: 80.0f,
		attack_speed: 0.0f,
		attack_spread: 6.7f,
		attack_recovery: 0.85f,
		attack_falloff_start: 15.0f,
		attack_falloff_end: 30.0f,
		attack_headshot: 2.0f,
		
		attack2_damage: 50.0f,
		attack2_range: 8.0f,
		attack2_speed: 80.0f,
		attack2_recovery: 0.85f,
		attack2_headshot: 2.0f,
		
		ability1_cooldown: 8.0f,
		ability1_damage: 30.0f,
		ability1_range: 20.0f,
		ability1_speed: 40.0f,
		ability1_cast_time: 0.3f,
		
		ability2_cooldown: 8.0f,
		
		ult_duration: 5.5f,
		ult_numshots: 128,
		ult_damage: 7.0f,
		ult_cast_time: 0.5f,
	},
	{
		name: "Sigma",
		
		value: 1000,
		
		color: 0x7359ba,
		
		view_height: 2.10f,
		
		shift_id: 0x5,
		e_id: 0x6,
		
		max_health: 300,
		max_shields: 100,
		
		attack_numshots: 2,
		attack_damage: 55,
		attack_splash_min: 9.0f,
		attack_splash_max: 30.0f,
		attack_splash_radius: 3.0f,
		attack_range: 22.0f,
		attack_speed: 50.0f,
		attack_recovery: 0.2f,
		
		attack2_cooldown: 2.0f,
		
		ability1_cooldown: 12.0f,
		ability1_range: 3.0f,
		
		ability2_cooldown: 10.0f,
		ability2_damage: 70.0f,
		ability2_splash_min: 12.0f,
		ability2_splash_max: 40.0f,
		ability2_splash_radius: 2.5f,
		ability2_speed: 37.5f,
		ability2_move_mult: -75.0f,
		ability2_cast_time: 0.65f,
		
		ult_duration: 5.0f,
		ult_range: 35.0f,
		ult_move_mult: 130.0f,
		ult_cast_time: 0.6f,
	},
	{
		name: "Monkey",
		
		value: 1000,
		
		color: 0xa2a6bf,
		
		shift_id: 0x6,
		e_id: 0x7,
		
		max_health: 350.0f,
		max_armor: 150.0f,
		
		attack_damage: 3,
		attack_splash_radius: 6.0f,
		attack_range: 8.0f,
		attack_recovery: 0.05,
		
		ability1_cooldown: 6.0f,
		ability1_splash_min: 1.0f,
		ability1_splash_max: 50.0f,
		ability1_splash_radius: 5.0f,
		ability1_range: 22.0f,
		ability1_move_mult: 535.0f,
		
		ability2_cooldown: 13.0f,
		
		ult_duration: 10.0f,
		ult_damage: 40.0f,
		ult_range: 4.0f,
		ult_move_mult: 30.0f,
	},
	{
		name: "Ham",
		
		value: 1000,
		
		color: 0xDA9142,
		
		view_height: 1.83f,
		
		shift_id: 0x1,
		e_id: 0xa,
		//m2_id: 0x7,
		//ctrl_id: 0x8,
		
		max_health: 500.0f,
		max_armor: 100.0f,
		
		attack_damage: 5.0f,
		attack_spread: 2.0f,
		attack_recovery: 0.04f,
		attack_falloff_start: 15.0f,
		attack_falloff_end: 25.0f,
		attack_headshot: 2.0f,
		
		attack2_cooldown: 5.0f,
		attack2_damage: 50.0f,
		attack2_range: 23.0f,
		attack2_move_mult: 100.0f,
		
		ability2_cooldown: 15.0f,
		
		ult_duration: 20.0f,
		ult_health: 50.0f,
		ult_damage: 130.0f,
	},
	{
		name: "Zarya",
		
		value: 1400,
		
		color: 0xe77eb6,
		
		view_height: 1.60f,
		
		shift_id: 0x5,
		e_id: 0x6,
		
		max_health: 200.0f,
		max_shields: 200.0f,
		
		attack_damage: 3.75,
		attack_range: 15.0f,
		attack_recovery: 0.05f,
		
		attack2_damage: 47.0f,
		attack2_splash_min: 9.4f,
		attack2_splash_max: 47.0f,
		attack2_splash_radius: 2.5f,
		attack2_speed: 25.0f,
		attack2_recovery: 1.0f,
		
		ability1_cooldown: 10.0f,
		
		ability2_cooldown: 8.0f,
		ability2_range: 30.0f,
		
		ult_duration: 4.0f,
		ult_damage: 5.0f,
		ult_splash_radius: 6.0f,
		ult_range: 15.0f,
		ult_speed: 25.0f,
	},
	{
		name: "Ashe",
		
		value: 2500,
		
		color: 0x67666A,
		
		view_height: 1.45f,
		
		shift_id: 0x6,
		e_id: 0x3,
		
		max_health: 200.0f,
		
		attack_damage: 75.0f,
		attack_recovery: 0.65f,
		attack_falloff_start: 20.0f,
		attack_falloff_end: 40.0f,
		attack_headshot: 2.0f,
		attack_move_mult: -25.0f,
		
		attack2_damage: 40.0f,
		attack2_spread: 1.85f,
		attack2_recovery: 0.25f,
		attack2_falloff_start: 20.0f,
		attack2_falloff_end: 40.0f,
		attack2_headshot: 2.0f,
		
		ability1_cooldown: 10.0f,
		ability1_numshots: 15.0f,
		ability1_damage: 6.0f,
		ability1_range: 9.4f,
		
		ability2_cooldown: 12.0f,
		ability2_damage: 100.0f,
		ability2_splash_min: 20.0f,
		ability2_splash_max: 50.0f,
		ability2_splash_radius: 5.0f,
		ability2_speed: 25.0f,
		
		ult_duration: 10.0f,
		ult_health: 1200.0f,
		ult_damage: 120.0f,
		ult_range: 40.0f,
	},
	{
		name: "Bastion",
		
		value: 100,
		
		color: 0x7c8f7b,
		
		view_height: 1.82f,
		
		max_health: 200.0f,
		max_armor: 100.0f,
		
		attack_damage: 20.0f,
		attack_spread: 1.2f,
		attack_recovery: 0.125f,
		attack_falloff_start: 25.0f,
		attack_falloff_end: 45.0f,
		
		attack2_damage: 15.0f,
		attack2_spread: 2.7f,
		attack2_falloff_start: 30.0f,
		attack2_falloff_end: 50.0f,
		
		ability2_cooldown: 1.0f,
		
		ult_duration: 8.0f,
		ult_damage: 205.0f,
		ult_splash_min: 42.0f,
		ult_splash_max: 140.0f,
	},
	{
		name: "Doomfist",
		
		value: 2000,
		
		color: 0x815049,
    
    view_height: 1.50f,
		
		shift_id: 0x6,
		e_id: 0x8,
		//m2_id: 0x5,
		
		max_health: 250.0f,
		
		attack_numshots: 11.0f,
		attack_damage: 6.0f,
		//attack_speed: 80.0f,
		attack_spread: 2.15f,
		attack_recovery: 0.333f,
		attack_falloff_start: 15.0f,
		attack_falloff_end: 30.0f,
		
		attack2_cooldown: 4.0f,
		attack2_damage: 100.0f,
		attack2_move_mult: -50.0f,
		
		ability1_cooldown: 6.0f,
		ability1_damage: 50.0f,
		
		ability2_cooldown: 6.0f,
		ability2_damage: 125.0f,
		
		ult_duration: 4.0f,
		ult_damage: 300.0f,
		ult_splash_min: 15.0f,
		ult_splash_max: 200.0f,
		ult_splash_radius: 8.0f
	},
	{
		name: "Echo",
		
		value: 1800,
		
		color: 0x9BCBF6,
		
		view_height: 1.70f,
		
		shift_id: 0x4,
		e_id: 0x6,
		m2_id: 0x7,
		
		max_health: 200.0f,
		
		attack_numshots: 3.0f,
		attack_damage: 17.0f,
		attack_speed: 75.0f,
		attack_recovery: 0.32f,
		attack_headshot: 2.0f,
		
		attack2_numshots: 3.0f,
		attack2_damage: 5.0f,
		attack2_splash_min: 25.0f,
		attack2_splash_max: 25.0f,
		attack2_splash_radius: 2.0f,
		attack2_speed: 50.0f,
		attack2_recovery: 0.065f,
		
		ability2_damage: 50.0f,
		ability2_range: 16.0f,
		ability2_speed: 75.0f
	},
	{
		name: "Genji",
		
		value: 2800,
		
		color: 0x97ef43,
		
		view_height: 1.20f,
		
		shift_id: 0x6,
		e_id: 0x7,
		
		max_health: 200.0f,
		
		attack_numshots: 3,
		attack_damage: 29.0f,
		attack_speed: 60.0f,
		attack_recovery: 0.88f,
		attack_headshot: 2.0f,
		
		attack2_numshots: 3,
		attack2_damage: 29.0f,
		attack2_speed: 60.0f,
		attack2_spread: 9.0f,
		attack2_recovery: 0.68f,
		attack2_headshot: 2.0f,
		
		ability1_cooldown: 8.0f,
		ability1_damage: 50.0f,
		ability1_range: 15.0f,
		
		ability2_cooldown: 8.0f,
		ability2_duration: 8.0f,
		
		ult_damage: 110.0f,
		ult_range: 5.0f,
		ult_move_mult: 30.0f,
	},
	{
		name: "Hanzo",
		
		value: 2500,
		
		color: 0xb9b48a,
		
		view_height: 1.35f,
		
		shift_id: 0x4,
		e_id: 0x5,
		// lunge = 0x6
		
		max_health: 200.0f,
		
		attack_damage: 125.0f,
		attack_speed: 110.0f,
		attack_recovery: 0.5f, // real is 0.5f
		attack_headshot: 2.0f,
		attack_move_mult: -30.0f,
		
		ability1_cooldown: 12.0f,
		ability1_duration: 6.0f,
		
		ability2_cooldown: 10.0f,
		ability2_damage: 70.0f,
		ability2_speed: 110.0f,
		ability2_headshot: 2.0f,
		
		ult_damage: 150.0f,
		ult_splash_radius: 4.0f,
		ult_speed: 20.0f,
	},
	{
		name: "Junk",
		
		value: 1000,
		
		color: 0xecbd53,
		
		shift_id: 0x6,
		e_id: 0x7,
		
		max_health: 200.0f,
		
		attack_damage: 120.0f,
		attack_splash_min: 10.0f,
		attack_splash_max: 80.0f,
		attack_splash_radius: 2.0f,
		attack_speed: 25.0f,
		attack_recovery: 0.667,
		
		ability1_cooldown: 8.0f,
		ability1_damage: 120.0f,
		ability1_splash_min: 30.0f,
		ability1_splash_max: 120.0f,
		ability1_splash_radius: 3.0f,
		ability1_speed: 120.0f,
		
		ability2_cooldown: 10.0f,
		ability2_damage: 80.0f,
		
		ult_duration: 10.0f,
		ult_damage: 600.0f,
		ult_splash_min: 50.0f,
		ult_splash_max: 600.0f,
		ult_splash_radius: 10.0f,
	},
	{
		name: "McCree",
		
		value: 2700,
		
		color: 0xae595c,
		
		view_height: 1.43f,
		
		shift_id: 0x6,
		e_id: 0x7,
		
		max_health: 225.0f,
		
		attack_damage: 70.0f,
		attack_recovery: 0.5f,
		attack_falloff_start: 20.0f,
		attack_falloff_end: 40.0f,
		attack_headshot: 2.0f,
		
		attack2_numshots: 6,
		attack2_damage: 50.0f,
		attack2_recovery: 0.13f,
		attack2_falloff_start: 20.0f,
		attack2_falloff_end: 40.0f,
		
		ability1_cooldown: 6.0f,
		ability1_range: 6.0f,
		
		ability2_cooldown: 10.0f,
		ability2_damage: 25.0f,
		ability2_splash_radius: 3.0f,
		ability2_range: 7.0f,
		ability2_speed: 30.0f,
		
		ult_duration: 6.2f,
		ult_damage: 100.0f,
		ult_range: 200.0f,
		ult_move_mult: -70.0f,
	},
	{
		name: "Mei",
		
		value: 1500,
		
		color: 0x6faced,
		
		view_height: 1.3f,
		
		shift_id: 0x4,
		e_id: 0x5,
		
		max_health: 250.0f,
		
		attack_damage: 2.75f,
		attack_range: 10.0f,
		attack_speed: 20.0f,
		attack_recovery: 20.0f,
		
		attack2_damage: 75.0f,
		attack2_speed: 115.0f,
		attack2_recovery: 0.8f,
		attack2_headshot: 2.0f,
		
		ability1_cooldown: 12.0f,
		ability1_duration: 4.0f,
		
		ability2_cooldown: 12.0f,
		ability2_range: 35.0f,
		
		ult_duration: 4.25f,
		ult_damage: 20.0f,
		ult_range: 15.0f,
	},
	{
		name: "Pharah",
		
		value: 1300,
		
		color: 0x3e7dca,
		
		view_height: 1.35f,
		
		shift_id: 0x2,
		e_id: 0x7,
		
		max_health: 200.0f,
		
		attack_damage: 120.0f,
		attack_splash_min: 24.6f,
		attack_splash_max: 80.0f,
		attack_splash_radius: 2.5f,
		attack_speed: 35.0f,
		attack_recovery: 0.85f,
		
		ability1_cooldown: 10.0f,
		
		ability2_cooldown: 9.0f,
		ability2_splash_radius: 8.0f,
		ability2_speed: 60.0f,
		
		ult_duration: 2.5f,
		ult_numshots: 30,
		ult_damage: 40.0f,
		ult_splash_min: 9.0f,
		ult_splash_max: 30.0f,
		ult_splash_radius: 2.0f,
		ult_speed: 45.0f,
	},
	{
		name: "Reaper",
		
		value: 1300,
		
		color: 0x7d3e51,
    
    view_height: 1.55f,
		
		shift_id: 0x1,
		e_id: 0x5,
		
		max_health: 250.0f,
		
		attack_numshots: 20.0f,
		attack_damage: 6.0f,
		attack_spread: 6.0f,
		attack_recovery: 0.5f,
		attack_falloff_start: 10.0f,
		attack_falloff_end: 20.0f,
		attack_headshot: 2.0f,
		
		ability1_cooldown: 8.0f,
		ability1_duration: 3.0f,
		ability1_move_mult: 50.0f,
		
		ability2_cooldown: 10.0f,
		ability2_range: 35.0f,
		
		ult_damage: 170.0f,
		ult_move_mult: -50.0f,
	},
	{
		name: "Soldier",
		
		value: 2000,
		
		color: 0x697794,
		
		view_height: 1.55f,
		
		shift_id: 0x0,
		e_id: 0x6,
		m2_id: 0xa,
		
		max_health: 200.0f,
		
		attack_damage: 20.0f,
		attack_recovery: 0.16f,
		//attack_recovery: 0.11111f,
		attack_falloff_start: 30.0f,
		attack_falloff_end: 50.0f,
		
		attack2_cooldown: 6.0f,
		attack2_damage: 120.0f,
		attack2_splash_min: 40.0f,
		attack2_splash_max: 80.0f,
		attack2_splash_radius: 3.0f,
		attack2_speed: 50.0f,
		
		ability1_move_mult: 50.0f,
		
		ability2_cooldown: 15.0f,
		ability2_duration: 5.0f,
		
		ult_duration: 6.0f,
		ult_cast_time: 1.2f,
	},
	{
		name: "Sombra",
		
		value: 1000,
		
		color: 0x7359ba,
    
    view_height: 1.30f,
		
		shift_id: 0x7,
		e_id: 0x6,
		m2_id: 0x9,
		
		max_health: 200.0f,
		
		attack_damage: 8.0f,
		attack_spread: 2.4f,
		attack_recovery: 0.05f,
		attack_falloff_start: 15.0f,
		attack_falloff_end: 35.0f,
		
		attack2_cooldown: 8.0f,
		attack2_range: 15.0f,
		
		ability1_cooldown: 6.0f,
		ability1_move_mult: 65.0f,
		
		ability2_cooldown: 6.0f,
		ability2_speed: 25.0f,
		
		ult_range: 15.0f,
	},
	{
		name: "Sym",
		
		value: 2000,
		
		color: 0x8ebccc,
		
		view_height: 1.30f,
		
		shift_id: 0x5,
		e_id: 0x7,
		
		max_health: 100.0f,
		max_shields: 125.0f,
		
		attack_damage: 60.0f,
		attack_range: 12.0f,
		
		attack2_damage: 120.0f,
		attack2_splash_min: 15.0f,
		attack2_splash_max: 60.0f,
		attack2_splash_radius: 2.0f,
		attack2_speed: 25.0f,
		attack2_recovery: 0.25f,
		
		ability1_cooldown: 10.0f,
		
		ability2_cooldown: 10.0f,
		
		ult_duration: 12.0f,
		ult_range: 25.0f,
	},
	{
		name: "Torbjorn",
		
		color: 0xc0726e,
		
		view_height: 1.0f,
		
		max_health: 200.0f,
		max_armor: 50.0f,
		
		attack_damage: 70.0f,
		attack_speed: 70.0f,
		attack_recovery: 0.6f,
		attack_headshot: 2.0f,
		
		attack2_numshots: 10,
		attack2_damage: 12.5f,
		attack2_speed: 120.0f,
		attack2_spread: 4.15f,
		attack2_recovery: 0.8f,
		attack2_falloff_start: 10.0f,
		attack2_falloff_end: 20.0f,
		attack2_headshot: 2.0f,
		
		ability1_cooldown: 5.0f,
		
		ability2_cooldown: 10.0f,
		ability2_duration: 5.0f,
		ability2_move_mult: 30.0f,
		
		ult_duration: 10.0f,
		ult_damage: 160.0f,
		ult_splash_radius: 2.5f,
		ult_speed: 40.0f,
	},
	{
		name: "Tracer",
		
		value: 3500,
		
		color: 0xd79342,
		
		view_height: 1.25f,
		
		shift_id: 0x2,
		e_id: 0x1,
		
		max_health: 150.0f,
		
		attack_numshots: 2.0f,
		attack_damage: 6.0f,
		attack_spread: 3.6f,
		attack_recovery: 0.05f,
		attack_falloff_start: 12.0f,
		attack_falloff_end: 20.0f,
		
		ability1_cooldown: 3.0f,
		ability1_range: 7.5f,
		
		ability2_cooldown: 12.0f,
		
		ult_duration: 1.15f,
		ult_damage: 5.0f,
		ult_splash_min: 70.0f,
		ult_splash_max: 350.0f,
		ult_range: 10.0f,
		ult_speed: 15.0f,
	},
	{
		name: "Widow",
		
		value: 4000,
		
		color: 0x9e6aa8,
		
		view_height: 1.50f,
		
		shift_id: 0x4,
		e_id: 0x3,
		
		max_health: 175.0f,
		
		attack_damage: 120.0f,
		attack_recovery: 0.5f,
		attack_falloff_start: 70.0f,
		attack_falloff_end: 120.0f,
		attack_headshot: 2.5f,
		attack_move_mult: -65.0f,
		
		attack2_damage: 13.0f,
		attack2_spread: 3.0f,
		attack2_recovery: 0.1f,
		attack2_falloff_start: 20.0f,
		attack2_falloff_end: 40.0f,
		attack2_headshot: 2.0f,
		
		ability1_cooldown: 12.0f,
		ability1_range: 20.0f,
		
		ability2_cooldown: 15.0f,
		ability2_duration: 5.0f,
		ability2_damage: 15.0f,
		ability2_splash_radius: 3.0f,
		ability2_speed: 20.0f,
		
		ult_duration: 15.0f,
	},
	{
		name: "Ana",
		
		value: 1000,
		
		color: 0x718ab3,
		
		view_height: 1.25f,
		
		shift_id: 0x6,
		e_id: 0x4,
		
		max_health: 200.0f,
		
		attack_damage: 70.0f,
		attack_recovery: 0.8f,
		attack_move_mult: -65.0f,
		
		attack2_damage: 70.0f,
		attack2_speed: 125.0f,
		attack2_recovery: 0.8f,
		
		ability1_cooldown: 12.0f,
		ability1_duration: 3.0f,
		ability1_damage: 5.0f,
		ability1_speed: 60.0f,
		
		ability2_cooldown: 10.0f,
		ability2_duration: 4.0f,
		ability2_damage: 60.0f,
		ability2_splash_radius: 4.0f,
		ability2_speed: 30.0f,
		
		ult_duration: 8.0f,
		ult_range: 40.0f,
	},
	{
		name: "Baptiste",
		
		value: 2000,
		
		color: 0x56B0CB,
    
    view_height: 1.35f,
		
		shift_id: 0x5,
		e_id: 0x4,
		
		max_health: 200.0f,
		
		attack_numshots: 3.0f,
		attack_damage: 24.0f,
		attack_recovery: 0.58f,
		attack_falloff_start: 25.0f,
		attack_falloff_end: 45.0f,
		attack_headshot: 2.0f,
		
		attack2_splash_radius: 3.0f,
		attack2_speed: 60.0f,
		attack2_recovery: 0.9f,
		
		ability1_cooldown: 13.0f,
		ability1_duration: 5.0f,
		ability1_splash_radius: 10.0f,
		
		ability2_cooldown: 25.0f,
		
		ult_duration: 10.0f,
		ult_range: 35.0f,
	},
	{
		name: "Brig",
		
		value: 2000,
		
		color: 0xbe736e,
		
		shift_id: 0x6,
		e_id: 0x7,
		m2_id: 0x4,
		
		max_health: 150.0f,
		max_armor: 50.0f,
		
		attack_damage: 35.0f,
		attack_range: 6.0f,
		
		attack2_cooldown: 5.0f,
		attack2_move_mult: -30.0f,
		
		ability1_cooldown: 4.0f,
		ability1_damage: 70.0f,
		ability1_range: 20.0f,
		ability1_speed: 80.0f,
		
		ability2_cooldown: 6.0f,
		
		ult_duration: 10.0f,
		ult_move_mult: 30.0f,
	},
	{
		name: "Lucio",
		
		value: 2000,
		
		color: 0x85c952,
		
		shift_id: 0x0, // 0x5
		e_id: 0x6,
		m2_id: 0x9,
		
		max_health: 200.0f,
		
		attack_numshots: 4,
		attack_damage: 20.0f,
		attack_speed: 50.0f,
		attack_recovery: 0.935f,
		
		attack2_cooldown: 4.0f,
		attack2_damage: 25.0f,
		attack2_range: 8.0f,
		
		ability2_cooldown: 12.0f,
		ability2_duration: 3.0f,
		
		ult_duration: 7.0f,
		ult_range: 30.0f,
	},
	{
		name: "Mercy",
		
		value: 2500,
		
		color: 0xebe8bb,
    
    view_height: 1.45f,
		
		shift_id: 0x3,
		e_id: 0x5,
		
		max_health: 200.0f,
		
		attack_damage: 20.0f,
		attack_speed: 50.0f,
		attack_recovery: 0.2f,
		
		ability1_cooldown: 1.5f,
		ability1_range: 30.0f,
		
		ability2_cooldown: 30.0f,
		ability2_range: 5.0f,
		
		ult_duration: 15.0f,
		ult_cast_time: 0.5f,
	},
	{
		name: "Moira",
		
		value: 2000,
		
		color: 0x803c51,
		
		shift_id: 0x4,
		e_id: 0x6,
		
		max_health: 200.0f,
		
		attack_range: 15.0f,
		
		attack2_damage: 50.0f,
		attack2_range: 20.0f,
		
		ability1_cooldown: 6.0f,
		ability1_range: 15.0f,
		ability1_move_mult: 250.0f,
		
		ability2_cooldown: 8.0f,
		ability2_damage: 50.0f,
		ability2_range: 4.0f,
		
		ult_duration: 8.0f,
		ult_damage: 70.0f,
		ult_range: 30.0f,
		ult_move_mult: 50.0f,
	},
	{
		name: "Zen",
		
		value: 2500,
		
		color: 0xede582,
    
    view_height: 1.55f,
		
		max_health: 50.0f,
		max_shields: 150.0f,
		
		attack_damage: 48.0f,
		attack_speed: 90.0f,
		attack_recovery: 0.4f,
		attack_headshot: 2.0f,
		
		ability1_duration: 3.0f,
		ability1_range: 40.0f,
		
		ability2_duration: 3.0f,
		ability2_range: 40.0f,
		
		ult_duration: 6.0f,
		ult_range: 10.0f,
		ult_move_mult: 100.0f,
	},
	{
		name: "Training Bot1",
		
		color: 0xFFFFFF,
	},
	{
		name: "Training Bot2",
		
		color: 0xFFFFFF,
	},
	{
		name: "Training Bot3",
		
		color: 0xFFFFFF,
	},
	{
		name: "Training Bot4",
		
		color: 0xFFFFFF,
	}
};

uint64_t g_pool_ids[128] = {
	POOLID_LOCALPLAYER,
	POOLID_PLAYER,
	POOLID_TRAININGBOT,
	POOLID_TRAININGBOT1,
	POOLID_TRAININGBOT4,
	POOLID_MCCREE_BOT,
	POOLID_REAPER_BOT,
	POOLID_TORB_BOT,
	POOLID_BASTION_BOT,
	POOLID_ZEN_BOT,
	POOLID_ROADHOG_BOT,
	POOLID_ZARYA_BOT,
	POOLID_SOLDIER_BOT,
	POOLID_LUCIO_BOT,
	POOLID_MEI_BOT,
	POOLID_SOMBRA_BOT,
	POOLID_ANA_BOT,
	
	POOLID_DVAMECH,
	POOLID_DVAPILOT,
	POOLID_ORISA,
	POOLID_REINHARDT,
	POOLID_ROADHOG,
	POOLID_SIGMA,
	POOLID_MONKEY,
	POOLID_HAMSTER,
	POOLID_ZARYA,
	POOLID_ASHE,
	POOLID_BASTION,
	POOLID_MCCREE,
	POOLID_DOOM,
	POOLID_ECHO,
	POOLID_GENJI,
	POOLID_HANZO,
	POOLID_JUNK,
	POOLID_MEI,
	POOLID_PHARAH,
	POOLID_REAPER,
	POOLID_SOLDIER,
	POOLID_SOMBRA,
	POOLID_SYM,
	POOLID_TORB,
	POOLID_TRACER,
	POOLID_WIDOW,
	POOLID_ANA,
	POOLID_BAPTISTE,
	POOLID_BRIG,
	POOLID_LUCIO,
	POOLID_MERCY,
	POOLID_MOIRA,
	POOLID_ZEN,
	
	POOLID_DYNAMITE,
	POOLID_TRANSLOCATOR,
	POOLID_TPENTER,
	POOLID_TPEXIT,
	POOLID_WIDOWMINE,
	POOLID_LAMP,
};