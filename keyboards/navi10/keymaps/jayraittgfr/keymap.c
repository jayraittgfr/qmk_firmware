/* Copyright 2019 Ethan Durrant (emdarcher)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include QMK_KEYBOARD_H

//create the tap type
typedef struct {
    bool is_press_action;
    int state;
} tap;

//tap dance states
enum {
    SINGLE_TAP = 1,
    SINGLE_HOLD = 2,
    DOUBLE_TAP = 3,
    TRIPLE_TAP = 4
};

//tap dance keys
enum {
    TAPPY_KEY = 0
};

//function to handle all the tap dances
int cur_dance(qk_tap_dance_state_t *state);

//functions for each tap dance
void tk_finished(qk_tap_dance_state_t *state, void *user_data);
void tk_reset(qk_tap_dance_state_t *state, void *user_data);

#define INDICATOR_LED   B5
#define TX_LED          D5
#define RX_LED          B0

#define _LC0    0
#define _ML1    1
#define _NX2    2
#define _IE3    3

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_LC0] = LAYOUT(/* Base*/
                TD(TAPPY_KEY),C(KC_Z),  C(KC_Y),
                C(KC_X),    C(KC_C),    C(KC_V),
                 
                            KC_UP,
                KC_LEFT,    KC_DOWN,    KC_RIGHT),
    [_ML1] = LAYOUT(/* media layer, hold tappy*/
                KC_TRNS,    KC_VOLD,    KC_VOLU,
                KC_MPRV,    KC_MPLY,    KC_MNXT,
                 
                            KC_NO,/*disable LED (easy first macro)*/
                KC_NO,      KC_NO,      KC_NO),  
    [_NX2] = LAYOUT(/* NX layer, single tappy */
                KC_TRNS,    C(KC_M),    C(S(KC_D)), 
                C(KC_E),    KC_F8,      C(KC_W),
                 
                            LCA(KC_T),
                LCA(KC_L),  LCA(KC_F),  LCA(KC_R)),           
    [_IE3] = LAYOUT(/* inline editing, double tappy */
                KC_TRNS,    KC_UP,      C(KC_Z),
                C(KC_X),    KC_DOWN,    C(KC_Y),
                 
                            C(KC_V),
                KC_LEFT,    KC_BSPC,    KC_RIGHT),
};

void matrix_init_user(void) {
    //init the Pro Micro on-board LEDs
    setPinOutput(TX_LED);
    setPinOutput(RX_LED);
    //set to off
    writePinHigh(TX_LED);
    writePinHigh(RX_LED);
}

//determine the current tap dance state
int cur_dance (qk_tap_dance_state_t *state){
    if(state->count == 1){
        //if a tap was registered
        if(!state->pressed){
            //if not still pressed, then was a single tap
            return SINGLE_TAP;
        } else {
            //if still pressed/held down, then it's a single hold
            return SINGLE_HOLD;
        }
    } else if(state->count == 2){
        //if tapped twice, set to double tap
        return DOUBLE_TAP;
    } else if(state->count == 3){
        //if tapped thrice, set to triple tap
        return TRIPLE_TAP;
    } else {
        return 8;
    }
}

//initialize the tap structure for the tap key
static tap tk_tap_state = {
    .is_press_action = true,
    .state = 0
};

//functions that control what our tap dance key does
void tk_finished(qk_tap_dance_state_t *state, void *user_data){
    tk_tap_state.state = cur_dance(state);
    switch(tk_tap_state.state){
        case SINGLE_TAP:
            //send desired key when tapped:
            //setting to the media layer
            if(layer_state_is(_NX2)){
                //if already active, toggle it to off
                layer_off(_NX2);
                //turn off the indicator LED
                //set LED HI to turn it off
                writePinHigh(INDICATOR_LED);
            } else {
                //turn on the media layer
                layer_on(_NX2);
                //turn on the indicator LED
                //set LED pin to LOW to turn it on
                writePinLow(INDICATOR_LED);
            }
            break;
        
        case SINGLE_HOLD:
            //set to desired layer when held:
            //setting to the function layer
            layer_on(_ML1);
            break;

        case DOUBLE_TAP:
            //set to desired layer when double tapped:
            //setting to the media layer
            if(layer_state_is(_IE3)){
                //if already active, toggle it to off
                layer_off(_IE3);
                //turn off the indicator LED
                //set LED HI to turn it off
                writePinHigh(INDICATOR_LED);
            } else {
                //turn on the media layer
                layer_on(_IE3);
                //turn on the indicator LED
                //set LED pin to LOW to turn it on
                writePinLow(INDICATOR_LED);
            }
            break;

        case TRIPLE_TAP:
            //reset all layers
            layer_clear();
            //set all LEDs off
            writePinHigh(TX_LED);
            writePinHigh(RX_LED);
            writePinHigh(INDICATOR_LED);
            break;
    }
}

void tk_reset(qk_tap_dance_state_t *state, void *user_data){
    //if held and released, leave the layer
    if(tk_tap_state.state == SINGLE_HOLD){
        layer_off(_ML1);
    }
    //reset the state
    tk_tap_state.state = 0; 
}

//associate the tap dance key with its functionality
qk_tap_dance_action_t tap_dance_actions[] = {
    [TAPPY_KEY] = ACTION_TAP_DANCE_FN_ADVANCED_TIME(NULL, tk_finished, tk_reset, 275)
};
