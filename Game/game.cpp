#define is_down(b) input->buttons[b].is_down
#define pressed(b) (input->buttons[b].is_down && input->buttons[b].changed)
#define released(b) (!input->buttons[b].is_down && input->buttons[b].changed)


float player_1_p, player_1_dp, player_2_p, player_2_dp; //velocity 
float arena_half_size_x = 85, arena_half_size_y = 45;
float player_half_size_x = 2.5, player_half_size_y = 12;
float ball_p_x, ball_p_y,  ball_dp_y, ball_half_size = 1; 
float ball_dp_x = 130;  //ball speed

int player_1_score, player_2_score;

internal void
simulate_player(float *p, float *dp, float ddp, float dt) {
	//PLAYERS
	ddp -= *dp * 10.f; //Introduce friction 

	//Equations to increase the speed gradually over time
	*p = *p + *dp * dt + ddp * dt * dt * .5f;
	*dp = *dp + ddp * dt;

	if (*p + player_half_size_y > arena_half_size_y) {
		*p = arena_half_size_y - player_half_size_y;
		*dp *= -0.5;//velocity should be set to zero when collided
	}
	else if (*p - player_half_size_y < -arena_half_size_y) {
		*p = -arena_half_size_y + player_half_size_y;
		*dp *= -0.5;//velocity should be set to zero when collided
	}
}

internal bool
aabb_vs_aabb(float p1x, float p1y, float hs1x, float hs1y,
	float p2x, float p2y, float hs2x, float hs2y) {

	return (p1x + hs1x > p2x - hs2x &&
		p1x - hs1x < p2x + hs2x &&
		p1y + hs1y > p2y - hs2y &&
		p1y - hs1y < p2y + hs2y);
}

enum Gamemode {
	GM_MENU,
	GM_GAMEPLAY,
};

Gamemode current_gamemode;
int hot_button;
bool enemy_is_ai;

internal void
simulate_game(Input* input, float dt) {
	// Variables for controlling ball speed increase
	static float ball_speed_increase_rate = 5.0f; // Rate at which ball speed increases per second
	static float time_since_score = 0.0f; // Time elapsed since the last score


	// Increase ball speed gradually over time
	ball_dp_x += ball_speed_increase_rate * dt;
	ball_dp_y += ball_speed_increase_rate * dt;
	
	draw_rect(0, 0, arena_half_size_x, arena_half_size_y, 0xE5E5E5); //Arena
	draw_arena_borders(arena_half_size_x, arena_half_size_y, 0xCCCCCC); //border

	if (current_gamemode == GM_GAMEPLAY) {

		//player 1
		float player_1_ddp = 0.f;//acceleration 
		if (is_down(BUTTON_UP)) player_1_ddp += 2000;
		if (is_down(BUTTON_DOWN)) player_1_ddp -= 2000;

		//player 2
		
		float player_2_ddp = 0.f;
		if (!enemy_is_ai) {
			if (is_down(BUTTON_W)) player_2_ddp += 2000;
			if (is_down(BUTTON_S)) player_2_ddp -= 2000;
		}
		else {
			player_2_ddp = (ball_p_y - player_2_p) * 100;
			if (player_2_ddp > 1300) player_2_ddp = 1300;
			if (player_2_ddp < -1300) player_2_ddp = -1300;
		}

		simulate_player(&player_1_p, &player_1_dp, player_1_ddp, dt); //player 1
		simulate_player(&player_2_p, &player_2_dp, player_2_ddp, dt); //player 2


		//Ball Physics 
		ball_p_x += ball_dp_x * dt;
		ball_p_y += ball_dp_y * dt;
		//draw_rect(player_pos_x, player_pos_y, 1, 1, 0x407072); //user controlled entity 

		//Simulate ball
		{
			//ball collision with the players
			if (aabb_vs_aabb(ball_p_x, ball_p_y, ball_half_size, ball_half_size, 80, player_1_p, player_half_size_x, player_half_size_y)) {

				ball_p_x = 80 - player_half_size_x - ball_half_size;
				ball_dp_x *= -1;
				//ball_dp_y = player_1_dp * .75;
				ball_dp_y = (ball_p_y - player_1_p) * 2 + player_1_dp * .75; //ball collision when it hits the player

			}
			else if (aabb_vs_aabb(ball_p_x, ball_p_y, ball_half_size, ball_half_size, -80, player_2_p, player_half_size_x, player_half_size_y)) {

				ball_p_x = -80 + player_half_size_x + ball_half_size;
				ball_dp_x *= -1;
				ball_dp_y = (ball_p_y - player_2_p) * 2 + player_2_dp * .75;
			}
			//ball collision with the top and bottom of the arena 
			if (ball_p_y + ball_half_size > arena_half_size_y) {
				ball_p_y = arena_half_size_y - ball_half_size;
				ball_dp_y *= -1;
			}
			else if (ball_p_y - ball_half_size < -arena_half_size_y) {
				ball_p_y = -arena_half_size_y + ball_half_size;
				ball_dp_y *= -1;
			}
			//ball collision with the left and right of the arena
			if (ball_p_x + ball_half_size > arena_half_size_x) {
				ball_dp_x *= -1;
				ball_dp_y = 0;
				ball_p_x = 0;
				ball_p_y = 0;
				player_1_score++;
				time_since_score = 0.0f; // Reset time since last score
			}
			else if (ball_p_x - ball_half_size < -arena_half_size_x) {
				ball_dp_x *= -1;
				ball_dp_y = 0;
				ball_p_x = 0;
				ball_p_y = 0;
				player_2_score++;
				time_since_score = 0.0f; // Reset time since last score
			}
		}

		//score system

		draw_number(player_1_score, -10, 40, 1.f, 0x000000);
		draw_number(player_2_score, 10, 40, 1.f, 0x000000);

		// Increment time since last score
		time_since_score += dt;

		// Increase ball speed every 8 seconds
		if (time_since_score >= 8.0f) {
			ball_speed_increase_rate += 2.0f;
			time_since_score = 0.0f; // Reset time since last score
		}

		//rendering
		draw_rect(ball_p_x, ball_p_y, ball_half_size, ball_half_size, 0x666666);

		draw_rect(80, player_1_p, player_half_size_x, player_half_size_y, 0x3366CC);
		draw_rect(-80, player_2_p, player_half_size_x, player_half_size_y, 0xCC3333);
	}
	else {
		if (pressed(BUTTON_LEFT) || pressed(BUTTON_RIGHT)) {
			hot_button = !hot_button;
		}
		if (pressed(BUTTON_ENTER)) {
			current_gamemode = GM_GAMEPLAY;
			enemy_is_ai = hot_button ? 0 : 1;
		}
		if (hot_button == 0) {
			draw_text("SINGLE PLAYER", -80, -10, 1, 0xff0000);
			draw_text("MULTIPLAYER", 15, -10, 1, 0xcccccc);
		}
		else {
			draw_text("SINGLE PLAYER", -80, -10, 1, 0xcccccc);
			draw_text("MULTIPLAYER", 15, -10, 1, 0xff0000);
		}
		draw_text("PING PONG GAME", -60, 40, 1.5, 0x000000);
	}
}