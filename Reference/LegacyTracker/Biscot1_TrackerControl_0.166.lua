-- @description Biscot1 TrackerControl 0.166 chronological FIFO diagnostics protocol-20261013 startup-loader synchronization; legacy-song-gate-two-step-cutoff-fix Safe-live-pattern-copy Start-Offset-32 low-latency Tracker-before-Core Song-only MIDI trigger Song-packages-project-pattern-overlay library-song-backup-startup-mixer-overlay reliable-song-save-aligned-F3-cursor-nearest-pattern-jump F1-F3-undo song-bank-0-99 F3-pattern-style-navigation sparse-song-steps-f4-parameter-list song-end-physical-order rebuild-bank-after-browser-load step-only-f2-f4-song-load-focus user-named-files-only titled-pattern-song-names clean-file-browser-and-safe-pattern-names bipolar-reverse-gate independent-song-transport-f6 f6-arrow-layout song-bank-constants-fix trigger-only-f6-and-real-occupancy overwrite-cancel-returns-to-file step-audition-song-copy-numbering replay-after-value-change bounded-pattern-song-fields unconditional-stop-on-exit pattern-init-resets-core-filter-state exact-0.109-gate F3-mute-solo-step-selection F3-selection-key-echo-fix F1-step-selection sidechain-source-amount
-- @author Lo-lo + GPT
-- Direct GMEM tracker controller with per-pattern MIDI note mapping.

local GMEM_NS="Biscot1TrackerBridge"
local LUA_MAGIC=20261101
local LUA_BASE=10000
local PLAY_STATUS=LUA_BASE+1500
local DIAG_MAGIC=20260725
local DIAG_VERSION=2
local DIAG_BASE=40000
local DIAG_SIZE=256
local DIAG_OP_BASE=DIAG_BASE+32
local DIAG_OP_STRIDE=48
local QUEUE_CAPACITY=262144
local PATTERN_START_STEP=LUA_BASE+1501
local MIDI_MAP_BASE=LUA_BASE+1600
local SONG_MIDI_MAP_BASE=LUA_BASE+1728
local MIDI_PATTERN_BASE=100000
local MIDI_PATTERN_HEADER=1008
local BANK_MAGIC=20260713
local BANK_RELEASE_ARP_BASE=96000
local BANK_CTRL_BASE=98000
local BANK_DIR_BASE=100000
local BANK_DATA_BASE=110000
local BANK_MAX_ADDRESS=9500000
local REPEAT_TRANSPOSE_BASE=13000
local RELEASE_ARP_CURRENT=19500
local PATTERN_BASE=9600000
local MAX_PATTERNS=1000
local MAX_STEPS=1024
local MAX_CELLS=128
local CELL_SIZE=2
local STEP_STRIDE=1+MAX_CELLS*CELL_SIZE
local SONG_MAGIC=20260714
local SONG_BASE=9900000
local SONG_HEADER_SIZE=3020
local SONG_MAX_STEPS=4096
local SONG_STEP_STRIDE=12
local SONG_START_OFFSET_DIVISIONS=32
local SONG_START_PACK_BASE=SONG_START_OFFSET_DIVISIONS+1
local DELAY_LIVE_BASE=19700
local DELAY_LIVE_STRIDE=5
local SIDECHAIN_LIVE_BASE=19720
local SIDECHAIN_LIVE_STRIDE=3
local SONG_MIXER_LIVE_BASE=19740 -- Vol1, Vol2, Vol3, mute mask, solo lane, serial, valid magic, Song-active (owned by Tracker)
local SONG_MIXER_MAGIC=20260722
local SONG_MIXER_HEADER_MAGIC=202607220 -- validates packed per-Song mixer in header cells 3018-3019
local SONG_LANE_STRIDE=SONG_MAX_STEPS*SONG_STEP_STRIDE
SONG_BANK_BASE=10100000
SONG_BANK_STRIDE=SONG_HEADER_SIZE+3*SONG_LANE_STRIDE
local TRACKER_FX_PATH="JS: Lo-lo - JSFX/Instruments/Biscot1/Biscot1 Tracker.jsfx"
local CORE_FX_PATH="JS: Lo-lo - JSFX/Instruments/Biscot1/Biscot1.jsfx"

local CORE_SLIDER_DEFAULTS={
  [1]=1,
  [2]=0,
  [3]=2,
  [4]=1,
  [5]=0,
  [6]=0,
  [7]=0,
  [8]=-6,
  [9]=0,
  [10]=0,
  [11]=0,
  [12]=0,
  [13]=0.18,
  [14]=0.18,
  [15]=0.18,
  [16]=1,
  [17]=1,
  [18]=1,
  [19]=0,
  [20]=0,
  [21]=0,
  [22]=2,
  [23]=2,
  [24]=2,
  [25]=0,
  [26]=0,
  [27]=0,
  [28]=0,
  [29]=0,
  [30]=0,
  [31]=0,
  [32]=0,
  [33]=0,
  [34]=0,
  [35]=0,
  [36]=0,
  [37]=0.12,
  [38]=0.12,
  [39]=0.12,
  [40]=0.7,
  [41]=0.7,
  [42]=0.7,
  [43]=0,
  [44]=0,
  [45]=0,
  [46]=0,
  [47]=0,
  [48]=0,
  [49]=0,
  [50]=0,
  [51]=0,
  [52]=0,
  [53]=0,
  [54]=0,
  [55]=0,
  [56]=0,
  [57]=0,
  [58]=0,
  [59]=0,
  [60]=0,
  [61]=0,
  [62]=0,
  [63]=0,
  [64]=0,
  [65]=0,
  [66]=0,
  [67]=0.12,
  [68]=0.12,
  [69]=0.12,
  [70]=0,
  [71]=0,
  [72]=0,
  [73]=0.12,
  [74]=0.12,
  [75]=0.12,
  [76]=0,
  [77]=0,
  [78]=0,
  [79]=16,
  [80]=16,
  [81]=16,
  [82]=0,
  [83]=0,
  [84]=0,
  [85]=0,
  [86]=0,
  [87]=0,
  [88]=0,
  [89]=0,
  [90]=0,
  [91]=2,
  [92]=2,
  [93]=2,
  [94]=0,
  [95]=0,
  [96]=0,
  [97]=0,
  [98]=0,
  [99]=0,
  [100]=0,
  [101]=0,
  [102]=0,
  [103]=0,
  [104]=0,
  [105]=0,
  [106]=0,
  [107]=0,
  [108]=1200,
  [109]=0,
  [110]=0,
  [111]=0,
  [112]=0,
  [113]=0,
  [114]=0,
  [115]=0,
  [116]=0,
  [117]=0,
  [118]=0.002,
  [119]=0.12,
  [120]=0.7,
  [121]=0.12,
  [122]=0,
  [123]=1,
  [124]=1,
  [125]=0,
  [126]=0,
  [127]=0,
  [128]=0,
  [129]=0,
  [130]=1,
  [131]=0,
  [132]=0,
  [133]=0,
  [134]=0,
  [135]=0,
  [136]=0,
  [137]=0,
  [138]=0,
  [139]=0,
  [140]=0,
  [141]=0,
  [142]=0,
  [143]=0,
  [144]=0,
  [145]=0,
  [146]=1,
  [147]=1,
  [148]=0,
  [149]=0,
  [150]=0,
  [151]=0,
  [152]=0,
  [153]=1,
  [154]=0,
  [155]=0,
  [156]=0,
  [157]=0,
  [158]=0,
  [159]=0,
  [160]=0,
  [161]=0,
  [162]=0,
  [163]=0,
  [164]=0,
  [165]=0,
  [166]=0,
  [167]=0,
  [168]=0,
  [169]=1,
  [170]=1,
  [171]=0,
  [172]=0,
  [173]=0,
  [174]=0,
  [175]=0,
  [176]=1,
  [177]=0,
  [178]=0,
  [179]=0,
  [180]=0,
  [181]=0,
  [182]=0,
  [183]=0,
  [184]=0,
  [185]=0,
  [186]=0,
  [187]=0,
  [188]=0,
  [189]=0,
  [190]=0,
}

function speak(s)
  s=tostring(s or "")
  if reaper.osara_outputMessage then reaper.osara_outputMessage(s) else reaper.ShowConsoleMsg(s.."\n") end
end
function clamp(v,a,b) v=tonumber(v) or a; if v<a then return a elseif v>b then return b end return v end
function clampi(v,a,b) return math.floor(clamp(v,a,b)+0.5) end
function note_name(n)
  local names={"C","C sharp","D","D sharp","E","F","F sharp","G","G sharp","A","A sharp","B"}
  n=clampi(n,0,127); return tostring(n).." "..names[(n%12)+1].." "..tostring(math.floor(n/12)-1)
end

local PARAMS = {
  {id=1,name="Note",default=60,min=0,max=127,step=1,kind="note",tab=1},
  {id=2,name="Gate",default=0.92,min=0.01,max=1,step=0.01,tab=1},
  {id=3,name="Previous value",default=1,min=1,max=1,step=1,labels={[1]="Previous value"},tab=1},
  {id=4,name="Curve",default=0,min=-100,max=100,step=0.01,tab=1,tracker_curve=true,decimals=2},
  {id=8,name="Curve Resolution",default=32,min=2,max=1024,step=2,tab=1,curve_resolution=true},
  {id=9,name="Curve Ratio",default=0,min=0,max=8,step=1,tab=1,curve_ratio=true,labels={[0]="Off",[1]="1 to 1",[2]="2 to 1",[3]="3 to 1",[4]="4 to 1",[5]="1 to 2",[6]="1 to 3",[7]="1 to 4",[8]="2 to 2"}},
  {id=6,name="Pause",default=1,min=1,max=1,step=1,labels={[1]="Pause"},tab=1},
  {id=2000,name="OP Mode Trigger",default=0.0,min=0.0,max=1.0,step=1,map={10,11,12},tab=1},
  {id=2001,name="OP Drift",default=0.18,min=0.0,max=1.0,step=0.01,map={13,14,15},tab=1},
  {id=2002,name="OP Level",default=1.0,min=0.0,max=1.0,step=0.001,map={16,17,18},tab=1},
  {id=2003,name="OP Pan",default=0.0,min=-1.0,max=1.0,step=0.01,map={19,20,21},tab=1},
  {id=2004,name="OP Wave",default=2.0,min=0.0,max=7.0,step=1,map={22,23,24},tab=1},
  {id=2005,name="OP Pitch",default=0.0,min=-96.0,max=96.0,step=0.01,map={25,26,27},tab=1},
  {id=2006,name="OP Fine Pitch",default=0.0,min=-100.0,max=100.0,step=0.1,map={28,29,30},tab=1},
  {id=2007,name="OP PWM",default=0.0,min=-64.0,max=64.0,step=0.1,map={31,32,33},tab=1},
  {id=2008,name="OP Attack",default=0.0,min=0.0,max=2.0,step=0.001,map={34,35,36},tab=1},
  {id=2009,name="OP Decay",default=0.12,min=0.0,max=2.0,step=0.001,map={37,38,39},tab=1},
  {id=2010,name="OP Sustain",default=0.7,min=0.0,max=1.0,step=0.001,map={40,41,42},tab=1},
  {id=2011,name="OP Release",default=0.0,min=0.0,max=3.0,step=0.001,map={43,44,45},tab=1},
  {id=2012,name="OP Portamento",default=0.0,min=0.0,max=2.0,step=0.001,map={46,47,48},tab=1},
  {id=2013,name="OP Pan Mod Depth",default=0.0,min=-1.0,max=1.0,step=0.01,map={49,50,51},tab=1},
  {id=2014,name="OP Pitch Mod Depth",default=0.0,min=-48.0,max=48.0,step=0.01,map={52,53,54},tab=1},
  {id=2015,name="OP PWM Mod Depth",default=0.0,min=-1.5,max=1.5,step=0.001,map={55,56,57},tab=1},
  {id=2016,name="OP LFO Rate Mod Target",default=0.0,min=0.0,max=3.0,step=1,map={58,59,60},tab=1},
  {id=2017,name="OP LFO Rate Mod Depth",default=0.0,min=-2.0,max=2.0,step=0.01,map={61,62,63},tab=1},
  {id=2018,name="OP Mod Env Attack",default=0.0,min=0.0,max=2.0,step=0.001,map={64,65,66},tab=1},
  {id=2019,name="OP Mod Env Decay",default=0.12,min=0.0,max=2.0,step=0.001,map={67,68,69},tab=1},
  {id=2020,name="OP Mod Env Sustain",default=0.0,min=0.0,max=1.0,step=0.001,map={70,71,72},tab=1},
  {id=2021,name="OP Mod Env Release",default=0.12,min=0.0,max=3.0,step=0.001,map={73,74,75},tab=1},
  {id=2022,name="OP Arp Mode",default=0.0,min=0.0,max=6.0,step=1,map={76,77,78},tab=1},
  {id=7,name="Sub Note",default=64,min=0,max=127,step=1,kind="note",tab=1,allow_multiple=true},
  {id=2023,name="OP Arp Rate",default=16.0,min=0.125,max=128.0,step=0.125,map={79,80,81},tab=1},
  {id=2024,name="OP Arp Octave Up",default=0.0,min=0.0,max=4.0,step=1.0,map={82,83,84},tab=1},
  {id=2025,name="OP Arp Octave Down",default=0.0,min=-4.0,max=0.0,step=1.0,map={85,86,87},tab=1},
  {id=2026,name="OP Arp Glide Depth",default=0.0,min=0.0,max=1.0,step=0.001,map={88,89,90},tab=1},
  {id=2027,name="OP Pitch Bend",default=2.0,min=0.0,max=96.0,step=1.0,map={91,92,93},tab=1},
  {id=2028,name="OP Ring From Op3",default=0.0,min=0.0,max=1.0,step=1,map={94,95,96},tab=1},
  {id=2029,name="OP Sync By Op3",default=0.0,min=0.0,max=1.0,step=1,map={97,98,99},tab=1},
  {id=2030,name="OP Test Bit",default=0.0,min=0.0,max=1.0,step=1,map={100,101,102},tab=1},
  {id=1006,name="Filter Input",default=0.0,min=-24.0,max=24.0,step=0.1,slider=6,tab=2},
  {id=1007,name="Filter Output",default=0.0,min=-24.0,max=24.0,step=0.01,slider=7,tab=2},
  {id=1108,name="Filter Cutoff",default=1200.0,min=-2047.0,max=2047.0,step=1.0,slider=108,tab=2},
  {id=1109,name="Filter Resonance",default=0.0,min=0.0,max=15.0,step=0.1,slider=109,tab=2},
  {id=1110,name="Filter Low Pass",default=1.0,min=0.0,max=1.0,step=1,slider=110,tab=2},
  {id=1111,name="Filter Band Pass",default=0.0,min=0.0,max=1.0,step=1,slider=111,tab=2},
  {id=1112,name="Filter High Pass",default=0.0,min=0.0,max=1.0,step=1,slider=112,tab=2},
  {id=1113,name="Filter Key Follow",default=0.0,min=-1.0,max=1.0,step=0.001,slider=113,tab=2},
  {id=1114,name="Op1 To Filter",default=1.0,min=0.0,max=1.0,step=1,slider=114,tab=2},
  {id=1115,name="Op2 To Filter",default=1.0,min=0.0,max=1.0,step=1,slider=115,tab=2},
  {id=1116,name="Op3 To Filter",default=1.0,min=0.0,max=1.0,step=1,slider=116,tab=2},
  {id=1117,name="Filter Env Amount",default=0.0,min=-2047.0,max=2047.0,step=1.0,slider=117,tab=2},
  {id=1118,name="Filter Attack",default=0.002,min=0.0,max=2.0,step=0.001,slider=118,tab=2},
  {id=1119,name="Filter Decay",default=0.12,min=0.0,max=2.0,step=0.001,slider=119,tab=2},
  {id=1120,name="Filter Sustain",default=0.7,min=0.0,max=1.0,step=0.001,slider=120,tab=2},
  {id=1121,name="Filter Release",default=0.12,min=0.0,max=3.0,step=0.001,slider=121,tab=2},
  {id=1122,name="LFO1 Mode",default=0.0,min=0.0,max=1.0,step=1,slider=122,tab=3},
  {id=1123,name="LFO1 Rate",default=1.0,min=0.125,max=512.0,step=0.125,slider=123,tab=3},
  {id=1124,name="LFO1 Wave",default=1.0,min=0.0,max=6.0,step=1,slider=124,tab=3},
  {id=1125,name="LFO1 Upper Squash",default=0.0,min=0.0,max=1.0,step=0.01,slider=125,tab=3},
  {id=1126,name="LFO1 Lower Squash",default=0.0,min=0.0,max=1.0,step=0.01,slider=126,tab=3},
  {id=1127,name="LFO1 Delay",default=0.0,min=0.0,max=10.0,step=0.001,slider=127,tab=3},
  {id=1128,name="LFO1 Smooth",default=0.0,min=0.0,max=1.0,step=0.001,slider=128,tab=3},
  {id=1129,name="LFO1 One Shot",default=0.0,min=0.0,max=1.0,step=1,slider=129,tab=3},
  {id=1130,name="LFO1 One Shot Length",default=1.0,min=0.0,max=1.0,step=0.001,slider=130,tab=3},
  {id=1131,name="LFO1 Phase",default=0.0,min=0.0,max=1.0,step=0.001,slider=131,tab=3},
  {id=1132,name="LFO1 Cutoff Depth",default=0.0,min=-1.0,max=1.0,step=0.001,slider=132,tab=3},
  {id=1133,name="LFO1 OP Pitch Depth",default=0.0,min=-96.0,max=96.0,step=0.01,map={133,134,135},tab=3},
  {id=1134,name="LFO1 OP Pitch Depth",default=0.0,min=-96.0,max=96.0,step=0.01,slider=134,tab=3,hidden=true},
  {id=1135,name="LFO1 OP Pitch Depth",default=0.0,min=-96.0,max=96.0,step=0.01,slider=135,tab=3,hidden=true},
  {id=1136,name="LFO1 OP PWM Depth",default=0.0,min=-1.5,max=1.5,step=0.001,map={136,137,138},tab=3},
  {id=1137,name="LFO1 OP PWM Depth",default=0.0,min=-1.5,max=1.5,step=0.001,slider=137,tab=3,hidden=true},
  {id=1138,name="LFO1 OP PWM Depth",default=0.0,min=-1.5,max=1.5,step=0.001,slider=138,tab=3,hidden=true},
  {id=1139,name="LFO1 OP Level Depth",default=0.0,min=-1.0,max=1.0,step=0.001,map={139,140,141},tab=3},
  {id=1140,name="LFO1 OP Level Depth",default=0.0,min=-1.0,max=1.0,step=0.001,slider=140,tab=3,hidden=true},
  {id=1141,name="LFO1 OP Level Depth",default=0.0,min=-1.0,max=1.0,step=0.001,slider=141,tab=3,hidden=true},
  {id=1142,name="LFO1 OP Pan Depth",default=0.0,min=-1.0,max=1.0,step=0.001,map={142,143,144},tab=3},
  {id=1143,name="LFO1 OP Pan Depth",default=0.0,min=-1.0,max=1.0,step=0.001,slider=143,tab=3,hidden=true},
  {id=1144,name="LFO1 OP Pan Depth",default=0.0,min=-1.0,max=1.0,step=0.001,slider=144,tab=3,hidden=true},
  {id=1145,name="LFO2 Mode",default=0.0,min=0.0,max=1.0,step=1,slider=145,tab=3},
  {id=1146,name="LFO2 Rate",default=1.0,min=0.125,max=512.0,step=0.125,slider=146,tab=3},
  {id=1147,name="LFO2 Wave",default=1.0,min=0.0,max=6.0,step=1,slider=147,tab=3},
  {id=1148,name="LFO2 Upper Squash",default=0.0,min=0.0,max=1.0,step=0.01,slider=148,tab=3},
  {id=1149,name="LFO2 Lower Squash",default=0.0,min=0.0,max=1.0,step=0.01,slider=149,tab=3},
  {id=1150,name="LFO2 Delay",default=0.0,min=0.0,max=10.0,step=0.001,slider=150,tab=3},
  {id=1151,name="LFO2 Smooth",default=0.0,min=0.0,max=1.0,step=0.001,slider=151,tab=3},
  {id=1152,name="LFO2 One Shot",default=0.0,min=0.0,max=1.0,step=1,slider=152,tab=3},
  {id=1153,name="LFO2 One Shot Length",default=1.0,min=0.0,max=1.0,step=0.001,slider=153,tab=3},
  {id=1154,name="LFO2 Phase",default=0.0,min=0.0,max=1.0,step=0.001,slider=154,tab=3},
  {id=1155,name="LFO2 Cutoff Depth",default=0.0,min=-1.0,max=1.0,step=0.001,slider=155,tab=3},
  {id=1156,name="LFO2 OP Pitch Depth",default=0.0,min=-96.0,max=96.0,step=0.01,map={156,157,158},tab=3},
  {id=1157,name="LFO2 OP Pitch Depth",default=0.0,min=-96.0,max=96.0,step=0.01,slider=157,tab=3,hidden=true},
  {id=1158,name="LFO2 OP Pitch Depth",default=0.0,min=-96.0,max=96.0,step=0.01,slider=158,tab=3,hidden=true},
  {id=1159,name="LFO2 OP PWM Depth",default=0.0,min=-1.5,max=1.5,step=0.001,map={159,160,161},tab=3},
  {id=1160,name="LFO2 OP PWM Depth",default=0.0,min=-1.5,max=1.5,step=0.001,slider=160,tab=3,hidden=true},
  {id=1161,name="LFO2 OP PWM Depth",default=0.0,min=-1.5,max=1.5,step=0.001,slider=161,tab=3,hidden=true},
  {id=1162,name="LFO2 OP Level Depth",default=0.0,min=-1.0,max=1.0,step=0.001,map={162,163,164},tab=3},
  {id=1163,name="LFO2 OP Level Depth",default=0.0,min=-1.0,max=1.0,step=0.001,slider=163,tab=3,hidden=true},
  {id=1164,name="LFO2 OP Level Depth",default=0.0,min=-1.0,max=1.0,step=0.001,slider=164,tab=3,hidden=true},
  {id=1165,name="LFO2 OP Pan Depth",default=0.0,min=-1.0,max=1.0,step=0.001,map={165,166,167},tab=3},
  {id=1166,name="LFO2 OP Pan Depth",default=0.0,min=-1.0,max=1.0,step=0.001,slider=166,tab=3,hidden=true},
  {id=1167,name="LFO2 OP Pan Depth",default=0.0,min=-1.0,max=1.0,step=0.001,slider=167,tab=3,hidden=true},
  {id=1168,name="LFO3 Mode",default=0.0,min=0.0,max=1.0,step=1,slider=168,tab=3},
  {id=1169,name="LFO3 Rate",default=1.0,min=0.125,max=512.0,step=0.125,slider=169,tab=3},
  {id=1170,name="LFO3 Wave",default=1.0,min=0.0,max=6.0,step=1,slider=170,tab=3},
  {id=1171,name="LFO3 Upper Squash",default=0.0,min=0.0,max=1.0,step=0.01,slider=171,tab=3},
  {id=1172,name="LFO3 Lower Squash",default=0.0,min=0.0,max=1.0,step=0.01,slider=172,tab=3},
  {id=1173,name="LFO3 Delay",default=0.0,min=0.0,max=10.0,step=0.001,slider=173,tab=3},
  {id=1174,name="LFO3 Smooth",default=0.0,min=0.0,max=1.0,step=0.001,slider=174,tab=3},
  {id=1175,name="LFO3 One Shot",default=0.0,min=0.0,max=1.0,step=1,slider=175,tab=3},
  {id=1176,name="LFO3 One Shot Length",default=1.0,min=0.0,max=1.0,step=0.001,slider=176,tab=3},
  {id=1177,name="LFO3 Phase",default=0.0,min=0.0,max=1.0,step=0.001,slider=177,tab=3},
  {id=1178,name="LFO3 Cutoff Depth",default=0.0,min=-1.0,max=1.0,step=0.001,slider=178,tab=3},
  {id=1179,name="LFO3 OP Pitch Depth",default=0.0,min=-96.0,max=96.0,step=0.01,map={179,180,181},tab=3},
  {id=1180,name="LFO3 OP Pitch Depth",default=0.0,min=-96.0,max=96.0,step=0.01,slider=180,tab=3,hidden=true},
  {id=1181,name="LFO3 OP Pitch Depth",default=0.0,min=-96.0,max=96.0,step=0.01,slider=181,tab=3,hidden=true},
  {id=1182,name="LFO3 OP PWM Depth",default=0.0,min=-1.5,max=1.5,step=0.001,map={182,183,184},tab=3},
  {id=1183,name="LFO3 OP PWM Depth",default=0.0,min=-1.5,max=1.5,step=0.001,slider=183,tab=3,hidden=true},
  {id=1184,name="LFO3 OP PWM Depth",default=0.0,min=-1.5,max=1.5,step=0.001,slider=184,tab=3,hidden=true},
  {id=1185,name="LFO3 OP Level Depth",default=0.0,min=-1.0,max=1.0,step=0.001,map={185,186,187},tab=3},
  {id=1186,name="LFO3 OP Level Depth",default=0.0,min=-1.0,max=1.0,step=0.001,slider=186,tab=3,hidden=true},
  {id=1187,name="LFO3 OP Level Depth",default=0.0,min=-1.0,max=1.0,step=0.001,slider=187,tab=3,hidden=true},
  {id=1188,name="LFO3 OP Pan Depth",default=0.0,min=-1.0,max=1.0,step=0.001,map={188,189,190},tab=3},
  {id=1189,name="LFO3 OP Pan Depth",default=0.0,min=-1.0,max=1.0,step=0.001,slider=189,tab=3,hidden=true},
  {id=1190,name="LFO3 OP Pan Depth",default=0.0,min=-1.0,max=1.0,step=0.001,slider=190,tab=3,hidden=true},
}
local PARAM_BY_ID={}; local PARAM_LISTS={{},{},{}}
for _,p in ipairs(PARAMS) do PARAM_BY_ID[p.id]=p; if not p.hidden then table.insert(PARAM_LISTS[p.tab],p) end end
-- Step-wide ordered metaparameters are available from every F2 section.
-- They keep the same IDs in the stored step, so OP, Filter and LFO share one
-- chronological row. Their effect begins only to their right.
for tab=2,3 do
  table.insert(PARAM_LISTS[tab],1,PARAM_BY_ID[6])
  table.insert(PARAM_LISTS[tab],1,PARAM_BY_ID[4])
  table.insert(PARAM_LISTS[tab],1,PARAM_BY_ID[3])
end

local patterns={}
local current_pattern=0
local current_step=-1
local current_cell=1
local logical_cell=1 -- remembered horizontal position across pattern steps
local current_tab=1
local current_param_index=1
local last_added_param_index={1,1,1}
local last_pattern_cell_by_tab={1,1,1}
local param_grid_height=8
local preview_op=1 -- F1 generic OP preview is always routed to Op1
local jump_sizes={1,5,10,15,20}
local jump_index=1
local page=1
local return_page_from_f2=1
-- Set only when Ctrl+Tab opens a Pattern from F3.  A second Ctrl+Tab on F1
-- returns to the exact Song, lane, step and parameter that originated the jump.
local song_pattern_return_context=nil
local playing=false
local pattern_play_from_cursor=false -- transient F1 P mode; never saved
local song_playing=false
local song_delay_prime_pending=0
local song_mixer_prime_pending=0
local song_space_last_time=-1000
local SONG_SPACE_DEBOUNCE=0.30
local song_play_start_time=-1000
local SONG_PLAY_STATUS_GRACE=0.60
local MAX_SONGS=100
local songs={}
local current_song=0
local function new_empty_song()
  return {length=16,lane_lengths={16,16,16},midi_note=-1,song_end=-1,chain_loop=0,song_loop_target=-1,lane_repeat={1,1,1},lane_speeds={{1},{1},{1}},filter_owner=1,filter_mask=7,lfo_owner={1,1,1},mixer_volumes={100,100,100},mixer_mute_mask=0,mixer_solo_lane=0,lanes={{},{},{}}}
end
local song=new_empty_song()
songs[0]=song
local song_lane=1
local song_last_operator_lane=1
local song_step=-1
local song_cell=1
-- Logical F3 cursor retained across sparse/empty Song rows.
local song_logical_param=nil
local song_logical_cell=1
local song_param_index=1
local SONG_PARAM_LIST={
  {id=1,name="Pattern"},{id=2,name="Speed"},{id=3,name="Transpose"},{id=4,name="Repeat"},
  {id=5,name="Pattern Start"},{id=6,name="Play Length"},{id=7,name="Next Pattern Step",info=true},{id=8,name="Pattern Remaining",info=true},{id=9,name="Legato"},
  {id=10,name="Filter Owner"},{id=11,name="Filter Operators"},{id=12,name="LFO1 Owner"},{id=13,name="LFO2 Owner"},{id=14,name="LFO3 Owner"},{id=15,name="Sidechain Source"},{id=16,name="Sidechain Amount"},{id=17,name="Delay Speed"},{id=18,name="Delay Feedback"},{id=19,name="Delay Volume"},{id=20,name="Delay Ping-Pong Depth"},
  {id=21,name="Pattern Position",info=true},{id=22,name="Start Offset"}
}
local F4_LIST={
  "Pattern","Start Offset","Speed","Transpose","Repeat","Legato",
  "Filter Owner","Filter Operators","LFO1 Owner","LFO2 Owner","LFO3 Owner","Sidechain Source","Sidechain Amount","Delay Speed","Delay Feedback","Delay Volume","Delay Ping-Pong Depth"
}
local F4_PARAM_IDS={1,22,2,3,4,9,10,11,12,13,14,15,16,17,18,19,20}
local f4_index=1
local lua_serial=0
local TAB_NAMES={"OP","Filter","LFO"}
local cell_clipboard=nil
local clipboard_enter_pending=false
local clipboard_pattern=nil
local clipboard_step=nil
local clipboard_tab=nil
local song_cell_clipboard=nil
local song_clipboard_enter_pending=false
local song_clipboard_song=nil
local song_clipboard_lane=nil
local song_clipboard_step=nil
local song_clipboard_param=nil
local whole_song_clipboard=nil
local whole_song_clipboard_source=nil
local pattern_step_selection_anchor=nil
local pattern_step_selection_end=nil
local pattern_selection_last_shift_time=0
local pattern_selection_last_shift_key=0
local pattern_steps_clipboard=nil
local pattern_clipboard_mode=nil -- "steps" or "parameter"; last successful Ctrl+C decides Ctrl+V
local whole_pattern_clipboard=nil
local whole_pattern_clipboard_source=nil

local song_step_selection_anchor=nil
local song_step_selection_end=nil
local song_selection_last_shift_time=0
local song_selection_last_shift_key=0
local song_steps_clipboard=nil
local song_steps_clipboard_lane=nil
local song_steps_clipboard_song=nil
local song_steps_clipboard_source_end=nil
local song_steps_enter_pending=false
local song_clipboard_mode=nil -- "steps" or "parameter"; last successful Ctrl+C decides Ctrl+V
local song_lane_muted={false,false,false}
local song_solo_lane=0
local song_mixer_lane=1

-- Independent undo histories for F1 Patterns and F3 Songs.
-- Each entry is a complete serialized state, so one Ctrl+Z reverses one edit.
local UNDO_LIMIT=128
local pattern_undo_stacks={}
local song_undo_stacks={}
local pattern_undo_baseline={}
local song_undo_baseline={}
local undo_restoring=false

local ignore_enter_until=0
local last_real_step=0
local step_recording_active=false
local step_recording_step=0
local step_record_sustain_down=false
local step_record_sustain_time=0
local last_recent_midi_seq=nil

function ensure_pattern(p)
  p=clampi(p,0,MAX_PATTERNS-1)
  if not patterns[p] then patterns[p]={length=16,speed=1.0,repeat_count=1,repeat_speeds={1.0},repeat_transposes={0},midi_note=-1,release_arp=0,steps={}} end
  if patterns[p].midi_note==nil then patterns[p].midi_note=-1 end
  if patterns[p].release_arp==nil then patterns[p].release_arp=0 end
  patterns[p].repeat_speeds=patterns[p].repeat_speeds or {}
  patterns[p].repeat_transposes=patterns[p].repeat_transposes or {}
  local rc=clampi(patterns[p].repeat_count or 1,1,1000)
  for i=1,rc do
    if patterns[p].repeat_speeds[i]==nil then patterns[p].repeat_speeds[i]=patterns[p].speed or 1.0 end
    if patterns[p].repeat_transposes[i]==nil then patterns[p].repeat_transposes[i]=0 end
  end
  local pat=patterns[p]
  for st=0,MAX_STEPS-1 do
    if not pat.steps[st] then pat.steps[st]={{},{},{}} end
  end
  return pat
end
function find_cell(cells,id) for i,c in ipairs(cells) do if c.id==id then return c,i end end end
local DEFAULT_OP_STEP={
  {id=1,value=60},      -- Note
  {id=2,value=0.92},    -- Gate
  {id=2004,value=2},    -- OP Wave
  {id=2002,value=1},    -- OP Level
  {id=2008,value=0},    -- OP Attack
  {id=2009,value=0.12}, -- OP Decay
  {id=2010,value=0.7},  -- OP Sustain
  {id=2011,value=0},    -- OP Release
}
local is_real_step
local refresh_pattern_after_edit

function cloned_default_cells(tab,step_index)
  local out={}
  if tab==1 then
    for _,d in ipairs(DEFAULT_OP_STEP) do
      -- Step 0 must never contain Previous value.
      if not (step_index==0 and d.id==3) then
        out[#out+1]={id=d.id,value=d.value}
      end
    end
  end
  return out
end
function ensure_default_step_parameters()
  local pat=ensure_pattern(0)
  for st=0,MAX_STEPS-1 do
    local cells=pat.steps[st][1]
    for _,d in ipairs(DEFAULT_OP_STEP) do
      if not find_cell(cells,d.id) then table.insert(cells,{id=d.id,value=d.value}) end
    end
  end
end
function reset_current_step_table()
  if not is_real_step() then return false end
  local pat=ensure_pattern(current_pattern)
  pat.steps[current_step][current_tab]=cloned_default_cells(current_tab,current_step)
  logical_cell=1; current_cell=1
  refresh_pattern_after_edit()
  speak("Step "..current_step.." "..TAB_NAMES[current_tab].." initialized")
  return true
end
function clear_current_pattern_table()
  local pat=ensure_pattern(current_pattern)
  for st=0,MAX_STEPS-1 do pat.steps[st][current_tab]={} end
  logical_cell=1; current_cell=1
  refresh_pattern_after_edit()
  speak("Pattern "..TAB_NAMES[current_tab].." cleared")
  return true
end
function reset_entire_current_pattern()
  local newpat={length=16,speed=1.0,repeat_count=1,repeat_speeds={1.0},repeat_transposes={0},midi_note=-1,release_arp=0,steps={}}
  for st=0,MAX_STEPS-1 do
    newpat.steps[st]={{},{},{}}
    if st<16 then newpat.steps[st][1]=cloned_default_cells(1,st) end
  end
  patterns[current_pattern]=newpat
  current_step=0; current_tab=1; logical_cell=1; current_cell=1
  -- A new Pattern must not inherit persistent synth state from the previous one.
  -- Restore the complete Core, including filter routing and Filter ADSR.
  reset_core_to_defaults()
  refresh_pattern_after_edit()
  speak("Pattern initialized. 16 steps. Repeat 1. Core reset")
  return true
end
local P_PATTERN_LENGTH=100
local P_PATTERN_REPEAT=101
local P_PATTERN_SPEED=102
local P_PATTERN_TRANSPOSE=105
local P_PATTERN_MIDI_NOTE=103
local P_STEP_RECORDING=104
local P_RELEASE_ARP=106
local META_LENGTH={id=P_PATTERN_LENGTH,name="Length",min=1,max=MAX_STEPS,step=1}
local META_SPEED={id=P_PATTERN_SPEED,name="Speed",min=0.25,max=16,step=0.25}
local META_TRANSPOSE={id=P_PATTERN_TRANSPOSE,name="Transpose",min=-96,max=96,step=1}
local META_REPEAT={id=P_PATTERN_REPEAT,name="Repeat",min=1,max=1000,step=1}
function pattern_length() return clampi(ensure_pattern(current_pattern).length or 16,1,MAX_STEPS) end
function is_header_row() return current_step==-1 end
function repeat_speed_row_count()
  return math.ceil(clampi(ensure_pattern(current_pattern).repeat_count or 1,1,1000)/5)
end
function is_repeat_row() return current_step==pattern_length() end
function is_speed_row() return current_step>pattern_length() and current_step<=pattern_length()+repeat_speed_row_count() end
function speed_row_number() return is_speed_row() and (current_step-pattern_length()) or 0 end
is_real_step=function() return current_step>=0 and current_step<pattern_length() end
function current_cells()
  if not is_real_step() then return {} end
  return ensure_pattern(current_pattern).steps[current_step][current_tab]
end
function meta_cell_count()
  local pat=ensure_pattern(current_pattern)
  if is_header_row() then return 3 end
  if is_repeat_row() then return 1 end
  if is_speed_row() then
    local first=(speed_row_number()-1)*5+1
    local repeats=math.max(0,math.min(5,clampi(pat.repeat_count or 1,1,1000)-first+1))
    return repeats*2
  end
  return 0
end
function current_cell_obj()
  local pat=ensure_pattern(current_pattern)
  if is_header_row() then
    current_cell=clampi(current_cell,1,3)
    if current_cell==1 then return {id=P_PATTERN_LENGTH,value=pat.length or 16,_meta=true} end
    if current_cell==2 then return {id=P_PATTERN_MIDI_NOTE,value=pat.midi_note or -1,_meta=true} end
    return {id=P_STEP_RECORDING,value=step_recording_active and 1 or 0,_meta=true}
  end
  if is_repeat_row() then
    local rc=clampi(pat.repeat_count or 1,1,1000)
    current_cell=1
    return {id=P_PATTERN_REPEAT,value=rc,_meta=true}
  end
  if is_speed_row() then
    local count=meta_cell_count()
    if count<=0 then return nil end
    current_cell=clampi(current_cell,1,count)
    local pair=math.floor((current_cell-1)/2)+1
    local ri=(speed_row_number()-1)*5+pair
    if current_cell%2==1 then
      return {id=P_PATTERN_SPEED,value=pat.repeat_speeds[ri] or 1.0,_meta=true,_repeat_speed=ri}
    end
    return {id=P_PATTERN_TRANSPOSE,value=pat.repeat_transposes[ri] or 0,_meta=true,_repeat_transpose=ri}
  end
  local cells=current_cells()
  if #cells==0 then return nil end
  current_cell=clampi(logical_cell,1,#cells)
  return cells[current_cell]
end
function value_text(c)
  if not c then return "empty" end
  local p=PARAM_BY_ID[c.id]
  if c.id==P_PATTERN_LENGTH then return tostring(clampi(c.value,1,MAX_STEPS)) end
  if c.id==P_PATTERN_MIDI_NOTE then
    local mv=tonumber(c.value) or -1
    if mv<0 then return "Off" end
    return note_name(mv)
  end
  if c.id==P_STEP_RECORDING then return (tonumber(c.value) or 0)>=0.5 and "On" or "Off" end
  if c.id==P_RELEASE_ARP then return (tonumber(c.value) or 0)>=0.5 and "On" or "Off" end
  if c.id==P_PATTERN_TRANSPOSE then return tostring(clampi(c.value,-96,96)) end
  if c.id==P_PATTERN_SPEED then
    local v=clamp(tonumber(c.value) or 1,0.25,16)
    local s=string.format("%.2f",v):gsub("0+$",""):gsub("%.$","")
    return s.."x"
  end
  if c.id==P_PATTERN_REPEAT then return tostring(clampi(c.value,1,1000)) end
  if not p then return tostring(c.value) end
  if p.kind=="note" then return note_name(c.value) end
  if p.labels then return p.labels[clampi(c.value,p.min,p.max)] or tostring(c.value) end
  local v=tonumber(c.value) or 0
  if p.tracker_curve then return string.format("%.2f",v) end
  if math.abs(v-math.floor(v+0.5))<1e-9 then return tostring(math.floor(v+0.5)) end
  return string.format("%.4f",v):gsub("0+$",""):gsub("%.$","")
end
local speak_cell
function pattern_prefix()
  return ""
end
function speak_position()
  local section=current_tab==1 and "Operator" or (current_tab==2 and "Filter" or "LFO")
  if is_header_row() or is_repeat_row() or is_speed_row() then
    local c=current_cell_obj()
    if not c then
      speak("Pattern"..tostring(current_pattern)..". "..section..", empty")
      return
    end
    local label
    if c.id==P_PATTERN_LENGTH then label="Length"
    elseif c.id==P_PATTERN_MIDI_NOTE then label="MIDI Note"
    elseif c.id==P_STEP_RECORDING then label="Step Recording"
    elseif c.id==P_RELEASE_ARP then label="Release Arpeggio"
    elseif c.id==P_PATTERN_REPEAT then label="Repeat"
    elseif c.id==P_PATTERN_SPEED then label=c._repeat_speed and ("Speed "..tostring(c._repeat_speed)) or "Speed"
    elseif c.id==P_PATTERN_TRANSPOSE then label=c._repeat_transpose and ("Transpose "..tostring(c._repeat_transpose)) or "Transpose"
    else
      local p=PARAM_BY_ID[c.id]
      label=p and p.name or "Parameter"
    end
    speak("Pattern"..tostring(current_pattern)..". "..section..", "..label..", "..value_text(c))
    return
  end
  local c=current_cell_obj()
  if not c then
    speak("Pattern"..tostring(current_pattern)..". "..section..", step "..tostring(current_step)..". empty")
    return
  end
  local p=PARAM_BY_ID[c.id]
  local label=(c.id==3) and "Previous value" or (p and p.name or "Parameter")
  speak("Pattern"..tostring(current_pattern)..". "..section..", step "..tostring(current_step)..". "..label..", "..value_text(c))
end
function speak_tab_position()
  speak_position()
end
speak_cell=function()
  local c=current_cell_obj(); if not c then speak(pattern_prefix().."empty") return end
  if c.id==P_PATTERN_LENGTH then speak("Length, "..value_text(c)); return end
  if c.id==P_PATTERN_MIDI_NOTE then speak("MIDI Note, "..value_text(c)); return end
  if c.id==P_STEP_RECORDING then speak("Step Recording, "..value_text(c)); return end
  if c.id==P_RELEASE_ARP then speak("Release Arpeggio, "..value_text(c)); return end
  if c.id==P_PATTERN_TRANSPOSE then
    if c._repeat_transpose then speak("Transpose "..c._repeat_transpose..", "..value_text(c))
    else speak("Transpose, "..value_text(c)) end
    return
  end
  if c.id==P_PATTERN_SPEED then
    if c._repeat_speed then speak("Speed "..c._repeat_speed..", "..value_text(c))
    else speak("Speed, "..value_text(c)) end
    return
  end
  if c.id==P_PATTERN_REPEAT then speak("Repeat, "..value_text(c)); return end
  local p=PARAM_BY_ID[c.id]
  if c.id==3 then speak("Previous value"); return end
  speak(pattern_prefix()..(p and p.name or "Parameter")..", "..value_text(c))
end
function speak_step_cell()
  if is_header_row() or is_repeat_row() or is_speed_row() then speak_cell(); return end
  local c=current_cell_obj()
  if not c then speak("step "..current_step..". empty") return end
  local p=PARAM_BY_ID[c.id]
  if c.id==3 then speak("step "..current_step..". Previous value"); return end
  speak("step "..current_step..". "..(p and p.name or "Parameter")..", "..value_text(c))
end
function resolved_id(c)
  local id=clampi(c and c.id or 0,0,9999)
  -- F1 stores an abstract Operator X. Older versions exposed separate Op1/2/3
  -- destinations on the LFO page, even though the Song already treated only
  -- the first member of every triplet as the generic Pattern destination.
  -- Normalize every legacy member here so old Patterns and new generic cells
  -- both address the current Pattern operator. The Song can then remap that
  -- generic destination to lane 1, 2 or 3 without changing the saved Pattern.
  local first=nil
  if id>=1133 and id<=1144 then first=1133+math.floor((id-1133)/3)*3
  elseif id>=1156 and id<=1167 then first=1156+math.floor((id-1156)/3)*3
  elseif id>=1179 and id<=1190 then first=1179+math.floor((id-1179)/3)*3 end
  if first then return first+(preview_op-1) end
  local p=PARAM_BY_ID[id]
  if not p then return id end
  if p.map then return 1000+p.map[preview_op] end
  return id
end
function step_base(st) return PATTERN_BASE+st*STEP_STRIDE end
local bank_serial=0
local rebuild_midi_pattern_bank
function pattern_has_content(pat)
  if not pat then return false end
  -- Pattern metadata is real persistent data. In particular, a MIDI keyboard
  -- assignment must keep the Pattern alive even when its step grid is empty,
  -- otherwise Song banks and Composition snapshots silently discard the map.
  if clampi(pat.midi_note or -1,-1,127)>=0 then return true end
  if clampi(pat.length or 16,1,MAX_STEPS)~=16 then return true end
  if clampi(pat.repeat_count or 1,1,1000)~=1 then return true end
  if clampi(pat.release_arp or 0,0,1)~=0 then return true end
  local rc=clampi(pat.repeat_count or 1,1,1000)
  for i=1,rc do
    if math.abs((pat.repeat_speeds and pat.repeat_speeds[i] or 1)-1)>1e-12 then return true end
    if clampi(pat.repeat_transposes and pat.repeat_transposes[i] or 0,-96,96)~=0 then return true end
  end
  for st=0,MAX_STEPS-1 do
    for tab=1,3 do
      if #(pat.steps[st][tab] or {})>0 then return true end
    end
  end
  return false
end

function write_pattern_memory(start_override)
  reaper.gmem_attach(GMEM_NS)
  local pat=ensure_pattern(current_pattern)
  for st=0,MAX_STEPS-1 do
    local flat={}
    for tab=1,3 do
      for _,c in ipairs(pat.steps[st][tab]) do
        if #flat<MAX_CELLS then flat[#flat+1]={id=resolved_id(c),value=tonumber(c.value) or 0} end
      end
    end
    local sb=step_base(st); reaper.gmem_write(sb,#flat)
    for i=1,MAX_CELLS do
      local cb=sb+1+(i-1)*CELL_SIZE; local c=flat[i]
      reaper.gmem_write(cb,c and c.id or 0); reaper.gmem_write(cb+1,c and c.value or 0)
    end
  end
  reaper.gmem_write(LUA_BASE+0,LUA_MAGIC)
  reaper.gmem_write(LUA_BASE+2,1)
  reaper.gmem_write(LUA_BASE+4,tonumber(pat.length) or 16)
  local rc=clampi(pat.repeat_count or 1,1,1000)
  reaper.gmem_write(LUA_BASE+5,rc)
  reaper.gmem_write(LUA_BASE+6,1.0)
  reaper.gmem_write(LUA_BASE+7,1) -- generic OP pattern always previews through Op1
  local cursor_step=is_real_step() and current_step or clampi(last_real_step or 0,0,math.max(0,(pat.length or 16)-1))
  local start_step=start_override
  if start_step==nil then start_step=pattern_play_from_cursor and cursor_step or 0 end
  start_step=clampi(start_step,0,math.max(0,(pat.length or 16)-1))
  reaper.gmem_write(PATTERN_START_STEP,start_step)
  reaper.gmem_write(RELEASE_ARP_CURRENT,clampi(pat.release_arp or 0,0,1))
  for i=1,1000 do
    reaper.gmem_write(LUA_BASE+7+i, i<=rc and clamp(tonumber(pat.repeat_speeds[i]) or 1.0,0.25,16) or 0)
    reaper.gmem_write(REPEAT_TRANSPOSE_BASE+i-1, i<=rc and clampi(pat.repeat_transposes[i] or 0,-96,96) or 0)
  end
end

function push_pattern(run,start_override)
  write_pattern_memory(start_override)
  lua_serial=lua_serial+1
  reaper.gmem_write(PLAY_STATUS,run and 1 or 0)
  reaper.gmem_write(LUA_BASE+3,run and 1 or 0)
  reaper.gmem_write(LUA_BASE+1,lua_serial) -- serial last
end

function preview_pattern_step(st)
  if st<0 or st>=pattern_length() then return end
  local pat=ensure_pattern(current_pattern)
  local flat={}
  for tab=1,3 do
    for _,c in ipairs(pat.steps[st][tab] or {}) do
      if #flat<MAX_CELLS then
        flat[#flat+1]={id=resolved_id(c),value=tonumber(c.value) or 0}
      end
    end
  end
  -- A step with no Note cannot produce a useful isolated audition.
  local has_note=false
  for _,c in ipairs(flat) do if c.id==1 then has_note=true break end end
  if not has_note then return end

  reaper.gmem_attach(GMEM_NS)
  -- Stop the previous audition before replacing the one-step buffer.
  lua_serial=lua_serial+1
  reaper.gmem_write(LUA_BASE+0,LUA_MAGIC)
  reaper.gmem_write(LUA_BASE+2,1)
  reaper.gmem_write(LUA_BASE+3,0)
  reaper.gmem_write(LUA_BASE+1,lua_serial)

  for s=0,MAX_STEPS-1 do
    local sb=step_base(s)
    if s==0 then
      reaper.gmem_write(sb,#flat)
      for i=1,MAX_CELLS do
        local cb=sb+1+(i-1)*CELL_SIZE
        local c=flat[i]
        reaper.gmem_write(cb,c and c.id or 0)
        reaper.gmem_write(cb+1,c and c.value or 0)
      end
    else
      reaper.gmem_write(sb,0)
    end
  end
  reaper.gmem_write(LUA_BASE+4,1)
  reaper.gmem_write(LUA_BASE+5,1)
  reaper.gmem_write(LUA_BASE+6,1)
  reaper.gmem_write(LUA_BASE+7,1)
  reaper.gmem_write(LUA_BASE+8,1)
  reaper.gmem_write(REPEAT_TRANSPOSE_BASE,0)
  reaper.gmem_write(RELEASE_ARP_CURRENT,0)
  lua_serial=lua_serial+1
  reaper.gmem_write(PLAY_STATUS,1)
  reaper.gmem_write(LUA_BASE+3,1)
  reaper.gmem_write(LUA_BASE+1,lua_serial)
end



local EXT_SECTION="Biscot1TrackerControlSession"
local PROJECT_EXT_SECTION="Biscot1TrackerProjectState"
local LIBRARY_EXT_SECTION="Biscot1TrackerLibrary"
local LIBRARY_EXT_KEY="root"
local f5_entries={}
local f5_index=1
local f5_path=""
local autosave_enabled=false

function join_path(a,b)
  local sep=package.config:sub(1,1)
  a=tostring(a or ""):gsub("[\\/]+$","")
  b=tostring(b or "")
  if a=="" then return b end
  if b=="" then return a end
  return a..sep..b
end
function script_dir()
  local info=debug and debug.getinfo and debug.getinfo(1,"S") or nil
  local source=info and tostring(info.source or "") or ""
  if source:sub(1,1)=="@" then source=source:sub(2) end
  source=source:gsub("\\","/")
  local dir=source:match("^(.*[/])") or (reaper.GetResourcePath().."/Scripts/Lo-lo - Scripts/Instruments/Biscot1 TrackerControl/")
  return dir:gsub("[\\/]+$","")
end
function ensure_dir(path)
  if reaper.RecursiveCreateDirectory then reaper.RecursiveCreateDirectory(path,0) end
end
function norm_path(path)
  return tostring(path or ""):gsub("\\","/"):gsub("/+","/"):gsub("/$","")
end
function parent_dir(path)
  path=norm_path(path)
  return path:match("^(.*)/[^/]+$") or ""
end
function base_name(path)
  path=norm_path(path)
  return path:match("([^/]+)$") or path
end
function project_directory()
  if not reaper.EnumProjects then return "" end
  local ok,_,path=pcall(reaper.EnumProjects,-1,"")
  if not ok then return "" end
  path=norm_path(path or "")
  if path=="" then return "" end
  if path:lower():match("%.rpp$") or path:lower():match("%.rpp%-bak$") then return parent_dir(path) end
  return path
end
function default_tracker_root_dir()
  local p=join_path(script_dir(),"Biscot1 Tracker")
  ensure_dir(p)
  return p
end
function project_tracker_root_dir()
  local pd=project_directory()
  if pd=="" then return "" end
  local p=join_path(pd,"Biscot1 Tracker")
  ensure_dir(p)
  return p
end
function suggested_library_parent()
  local home=os.getenv("USERPROFILE") or os.getenv("HOME") or ""
  if home~="" then return join_path(home,"Documents") end
  return reaper.GetResourcePath()
end
function library_root_dir()
  local p=reaper.GetExtState and (reaper.GetExtState(LIBRARY_EXT_SECTION,LIBRARY_EXT_KEY) or "") or ""
  p=norm_path(p)
  if p~="" then
    ensure_dir(p)
    -- Keep the central Library structure complete even for Libraries created
    -- by older controller versions.  Song is a first-class Library folder,
    -- just like Patterns and Compositions.
    ensure_dir(join_path(p,"Patterns"))
    ensure_dir(join_path(p,"Song"))
    ensure_dir(join_path(p,"Compositions"))
  end
  return p
end
function set_library_root_dir(chosen)
  chosen=norm_path(chosen)
  if chosen=="" then return false end
  -- The selected folder is a container. The actual library always has the
  -- plug-in name, unless the user selected that folder itself.
  local root=chosen
  if base_name(root):lower()~="biscot1 tracker" then root=join_path(root,"Biscot1 Tracker") end
  ensure_dir(root)
  ensure_dir(join_path(root,"Patterns"))
  ensure_dir(join_path(root,"Song"))
  ensure_dir(join_path(root,"Compositions"))
  if reaper.SetExtState then reaper.SetExtState(LIBRARY_EXT_SECTION,LIBRARY_EXT_KEY,norm_path(root),true) end
  return true
end
function choose_library_root()
  local existing=library_root_dir()
  local initial=existing~="" and parent_dir(existing) or suggested_library_parent()
  local chosen=""
  if reaper.JS_Dialog_BrowseForFolder then
    local ok,path=reaper.JS_Dialog_BrowseForFolder("Choose the folder that will contain Biscot1 Tracker",initial)
    if ok==1 or ok==true then chosen=path or "" end
  else
    local ok,path=reaper.GetUserInputs("Biscot1 Tracker Library",1,"Parent folder path:",initial)
    if ok then chosen=path or "" end
  end
  chosen=norm_path(chosen)
  if chosen=="" then return false end
  return set_library_root_dir(chosen)
end
function ensure_library_root()
  local p=library_root_dir()
  if p~="" then return true end
  return choose_library_root()
end
function library_patterns_dir()
  local root=library_root_dir()
  if root=="" then return "" end
  local p=join_path(root,"Patterns")
  ensure_dir(p)
  return p
end
function library_songs_dir()
  local root=library_root_dir()
  if root=="" then return "" end
  local p=join_path(root,"Song")
  ensure_dir(p)
  return p
end
function library_compositions_dir()
  local root=library_root_dir()
  if root=="" then return "" end
  local p=join_path(root,"Compositions")
  ensure_dir(p)
  return p
end
function active_song_marker_path(root) return join_path(root,"active_song_folder.txt") end
function read_marker(root)
  local f=io.open(active_song_marker_path(root),"rb")
  if not f then return "" end
  local t=(f:read("*a") or ""):gsub("[\r\n]","")
  f:close(); return t
end
function song_root_for(root,create)
  local name=read_marker(root)
  if name=="" then name="Song_001" end
  local p=join_path(root,name)
  if create then
    ensure_dir(p)
    local f=io.open(active_song_marker_path(root),"w")
    if f then f:write(name); f:close() end
  end
  return p
end
function tracker_root_dir()
  local pr=project_tracker_root_dir()
  if pr~="" then return pr end
  return default_tracker_root_dir()
end
function song_root_dir()
  return song_root_for(tracker_root_dir(),true)
end
function patterns_dir()
  local p=join_path(song_root_dir(),"Patterns")
  ensure_dir(p)
  return p
end

function songs_dir()
  local p=join_path(song_root_dir(),"Song")
  ensure_dir(p)
  return p
end
function compositions_dir()
  local p=join_path(song_root_dir(),"Compositions")
  ensure_dir(p)
  return p
end
function active_song_file_path()
  return join_path(song_root_dir(),"active_song_file.txt")
end
function active_pattern_path() return join_path(song_root_dir(),"active_pattern.txt") end
function active_pattern_file_path() return join_path(song_root_dir(),"active_pattern_file.txt") end
function pattern_path(p) return join_path(patterns_dir(),string.format("Pattern-%03d.txt",clampi(p or current_pattern,0,MAX_PATTERNS-1))) end
function sanitize_pattern_filename(name)
  name=tostring(name or ""):gsub('[\\/:*?"<>|]',"-"):gsub("^%s+",""):gsub("%s+$","")
  name=name:gsub("%.txt$","")
  if name=="" then return "" end
  if not name:lower():match("^pattern%-") then name="Pattern-"..name end
  return name..".txt"
end
function file_exists(path)
  local f=io.open(path,"rb")
  if f then f:close(); return true end
  return false
end
function automatic_pattern_filename()
  local stamp=os.date("%Y%m%d-%H%M%S")
  local base="Pattern-"..stamp
  local n=1
  local name
  repeat
    name=string.format("%s-%03d.txt",base,n)
    n=n+1
  until not file_exists(join_path(patterns_dir(),name))
  return name
end
function unique_pattern_filename(name)
  name=sanitize_pattern_filename(name)
  if name=="" then name=automatic_pattern_filename() end
  local path=join_path(patterns_dir(),name)
  if not file_exists(path) then return name end
  local stem=name:gsub("%.txt$","")
  local n=2
  repeat
    name=string.format("%s-%03d.txt",stem,n)
    path=join_path(patterns_dir(),name)
    n=n+1
  until not file_exists(path)
  return name
end
function normalize_text_newlines(t)
  return tostring(t or ""):gsub("\r\n","\n"):gsub("\r","\n")
end
function write_text_file(path,lines)
  -- Binary mode keeps the exact LF representation on Windows too.  The old
  -- text-mode writer translated LF to CRLF, while the safety verification read
  -- the file in binary mode and consequently reported a successful save as
  -- failed.
  local f=io.open(path,"wb")
  if not f then return false end
  if type(lines)=="table" then for _,line in ipairs(lines) do f:write(line,"\n") end else f:write(tostring(lines or "")) end
  f:close(); return true
end
function read_text_file(path)
  local f=io.open(path,"rb"); if not f then return nil end
  local t=f:read("*a") or ""; f:close(); return normalize_text_newlines(t)
end
function save_session_state()
  if not reaper.SetExtState then return end
  local vals={current_pattern,current_step,current_cell,logical_cell,current_tab,current_param_index,param_grid_height,jump_index,page,f5_index,
    last_added_param_index[1] or 1,last_added_param_index[2] or 1,last_added_param_index[3] or 1,
    last_pattern_cell_by_tab[1] or 1,last_pattern_cell_by_tab[2] or 1,last_pattern_cell_by_tab[3] or 1,current_song}
  reaper.SetExtState(EXT_SECTION,"state",table.concat(vals,","),false)
end
function load_session_state()
  if not reaper.GetExtState then return end
  local t=reaper.GetExtState(EXT_SECTION,"state") or ""
  local v={}; for n in t:gmatch("[^,]+") do v[#v+1]=tonumber(n) end
  if #v>=10 then
    current_pattern=clampi(v[1] or 0,0,MAX_PATTERNS-1)
    current_step=clampi(v[2] or -1,-1,MAX_STEPS)
    current_cell=math.max(1,clampi(v[3] or 1,1,MAX_CELLS))
    logical_cell=math.max(1,clampi(v[4] or 1,1,MAX_CELLS))
    current_tab=clampi(v[5] or 1,1,3)
    current_param_index=math.max(1,clampi(v[6] or 1,1,10000))
    param_grid_height=clampi(v[7] or 8,1,100)
    jump_index=clampi(v[8] or 1,1,#jump_sizes)
    page=clampi(v[9] or 1,1,6)
    f5_index=math.max(1,clampi(v[10] or 1,1,MAX_PATTERNS))
    last_added_param_index={v[11] or 1,v[12] or 1,v[13] or 1}
    last_pattern_cell_by_tab={v[14] or 1,v[15] or 1,v[16] or 1}
    current_song=clampi(v[17] or 0,0,MAX_SONGS-1)
  end
end
function save_pattern_txt(p,custom_filename)
  p=clampi(p or current_pattern,0,MAX_PATTERNS-1)
  local pat=ensure_pattern(p)
  local lines={"BISCOT1_TRACKER_PATTERN_TXT_V1","pattern="..p,"length="..clampi(pat.length or 16,1,MAX_STEPS),"repeat="..clampi(pat.repeat_count or 1,1,1000),"midi_note="..clampi(pat.midi_note or -1,-1,127),"release_arp="..clampi(pat.release_arp or 0,0,1)}
  local rc=clampi(pat.repeat_count or 1,1,1000)
  for i=1,rc do
    lines[#lines+1]="speed\t"..i.."\t"..tostring(clamp(tonumber(pat.repeat_speeds[i]) or 1,0.25,16))
    lines[#lines+1]="transpose\t"..i.."\t"..tostring(clampi(pat.repeat_transposes[i] or 0,-96,96))
  end
  for st=0,MAX_STEPS-1 do
    for tab=1,3 do
      local cells=pat.steps[st][tab] or {}
      if #cells>0 then
        lines[#lines+1]="step\t"..st.."\t"..tab
        for _,c in ipairs(cells) do lines[#lines+1]="cell\t"..tostring(c.id).."\t"..tostring(c.value) end
      end
    end
  end
  local target=custom_filename and join_path(patterns_dir(),custom_filename) or pattern_path(p)
  local ok=write_text_file(target,lines)
  if ok then
    write_text_file(active_pattern_path(),tostring(p))
    write_text_file(active_pattern_file_path(),norm_path(target))
    if reaper.SetProjExtState then
      reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"active_pattern",tostring(p))
      reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"active_pattern_file",norm_path(target))
      reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"active_pattern_data",biscot_serialize_pattern and biscot_serialize_pattern(p) or table.concat(lines,"\n"))
    end
  end
  return ok,target
end
local restore_controller_focus_after_dialog
function save_pattern_with_name()
  local suggested=automatic_pattern_filename()
  local ok,input=reaper.GetUserInputs("Save pattern",1,"Pattern name:",suggested:gsub("%.txt$",""))
  restore_controller_focus_after_dialog()
  if not ok then return false,"" end
  ensure_library_root()
  local filename=sanitize_name(input,true,nil)
  if filename=="" then return false,"" end
  local saved,container_path=save_pattern_txt(current_pattern,filename)
  if not saved then return false,container_path end
  local libname=filename
  local libpath=join_path(library_patterns_dir(),libname)
  write_text_file(libpath,biscot_serialize_pattern(current_pattern))
  set_pattern_name(current_pattern,filename)
  update_controller_title()
  return true,libpath
end
function load_pattern_txt(path,target)
  local f=io.open(path,"r"); if not f then return false end
  local p=clampi(target or tonumber(path:match("Pattern[-_](%d+)%.txt$")) or 0,0,MAX_PATTERNS-1)
  local pat={length=16,speed=1,repeat_count=1,repeat_speeds={1},repeat_transposes={0},midi_note=-1,release_arp=0,steps={}}
  for st=0,MAX_STEPS-1 do pat.steps[st]={{},{},{}} end
  local curst,curtab=nil,nil
  local preserve_target=(target~=nil)
  for line in f:lines() do
    if line:match("^pattern=") then
      if not preserve_target then p=clampi(tonumber(line:match("^pattern=(%-?%d+)")) or p,0,MAX_PATTERNS-1) end
    elseif line:match("^length=") then pat.length=clampi(tonumber(line:match("^length=(%-?%d+)")) or 16,1,MAX_STEPS)
    elseif line:match("^repeat=") then pat.repeat_count=clampi(tonumber(line:match("^repeat=(%-?%d+)")) or 1,1,1000)
    elseif line:match("^midi_note=") then pat.midi_note=clampi(tonumber(line:match("^midi_note=(%-?%d+)")) or -1,-1,127)
    elseif line:match("^release_arp=") then pat.release_arp=clampi(tonumber(line:match("^release_arp=(%-?%d+)")) or 0,0,1)
    elseif line:match("^speed\t") then
      local i,v=line:match("^speed\t([^\t]+)\t([^\t]+)"); i=tonumber(i); v=tonumber(v)
      if i then pat.repeat_speeds[i]=clamp(v or 1,0.25,16) end
    elseif line:match("^transpose\t") then
      local i,v=line:match("^transpose\t([^\t]+)\t([^\t]+)"); i=tonumber(i); v=tonumber(v)
      if i then pat.repeat_transposes[i]=clampi(v or 0,-96,96) end
    elseif line:match("^step\t") then
      local a,b=line:match("^step\t([^\t]+)\t([^\t]+)"); curst=clampi(tonumber(a) or 0,0,MAX_STEPS-1); curtab=clampi(tonumber(b) or 1,1,3)
    elseif line:match("^cell\t") and curst~=nil and curtab then
      local a,b=line:match("^cell\t([^\t]+)\t([^\t]+)")
      if a then table.insert(pat.steps[curst][curtab],{id=tonumber(a) or 0,value=tonumber(b) or 0}) end
    end
  end
  f:close()
  for i=1,pat.repeat_count do
    if pat.repeat_speeds[i]==nil then pat.repeat_speeds[i]=1 end
    if pat.repeat_transposes[i]==nil then pat.repeat_transposes[i]=0 end
  end
  patterns[p]=pat
  return true,p
end
rebuild_midi_pattern_bank=function()
  reaper.gmem_attach(GMEM_NS)
  for p=0,MAX_PATTERNS-1 do reaper.gmem_write(BANK_DIR_BASE+p,0); reaper.gmem_write(BANK_RELEASE_ARP_BASE+p,0) end
  for n=0,127 do reaper.gmem_write(MIDI_MAP_BASE+n,0) end
  local cursor=BANK_DATA_BASE
  local count=0
  local note_owner={}
  for p=0,MAX_PATTERNS-1 do
    local pat=patterns[p]
    if pat and pattern_has_content(pat) then
      local rc=clampi(pat.repeat_count or 1,1,1000)
      local base=cursor
      local step_dir_rel=5+rc*2
      reaper.gmem_write(base+0,BANK_MAGIC)
      reaper.gmem_write(base+1,p)
      reaper.gmem_write(base+2,clampi(pat.length or 16,1,MAX_STEPS))
      reaper.gmem_write(base+3,rc)
      reaper.gmem_write(base+4,step_dir_rel)
      for i=1,rc do
        reaper.gmem_write(base+4+i,clamp(tonumber(pat.repeat_speeds[i]) or 1,0.25,16))
        reaper.gmem_write(base+4+rc+i,clampi(pat.repeat_transposes[i] or 0,-96,96))
      end
      cursor=base+step_dir_rel+MAX_STEPS
      for st=0,MAX_STEPS-1 do
        local flat={}
        for tab=1,3 do for _,c in ipairs(pat.steps[st][tab] or {}) do
          if #flat<MAX_CELLS then flat[#flat+1]={id=resolved_id(c),value=tonumber(c.value) or 0} end
        end end
        reaper.gmem_write(base+step_dir_rel+st,cursor-base)
        reaper.gmem_write(cursor,#flat); cursor=cursor+1
        for _,c in ipairs(flat) do reaper.gmem_write(cursor,c.id); reaper.gmem_write(cursor+1,c.value); cursor=cursor+2 end
      end
      if cursor>=BANK_MAX_ADDRESS then break end
      reaper.gmem_write(BANK_DIR_BASE+p,base)
      reaper.gmem_write(BANK_RELEASE_ARP_BASE+p,clampi(pat.release_arp or 0,0,1))
      local note=clampi(pat.midi_note or -1,-1,127)
      if note>=0 and (note_owner[note]==nil or p==current_pattern) then note_owner[note]=p end
      count=count+1
    end
  end
  for note,p in pairs(note_owner) do reaper.gmem_write(MIDI_MAP_BASE+note,p+1) end
  bank_serial=math.floor(reaper.gmem_read(BANK_CTRL_BASE+1) or 0)+1
  reaper.gmem_write(BANK_CTRL_BASE+0,BANK_MAGIC)
  reaper.gmem_write(BANK_CTRL_BASE+2,count)
  reaper.gmem_write(BANK_CTRL_BASE+3,cursor)
  reaper.gmem_write(BANK_CTRL_BASE+1,bank_serial)
end

function pattern_number_from_txt(path)
  local txt=read_text_file(path) or ""
  local n=tonumber(txt:match("pattern=(%-?%d+)"))
  if n==nil then n=tonumber(base_name(path):match("Pattern_(%d+)%.txt$")) end
  if n==nil then return nil end
  return clampi(n,0,MAX_PATTERNS-1)
end
local ram_pattern_loaded={}
local pattern_names={}
local song_names={}
function display_file_name(path)
  local n=base_name(path or ""):gsub("%.txt$","")
  return n
end
function set_pattern_name(p,path_or_name)
  p=clampi(p or current_pattern,0,MAX_PATTERNS-1)
  local n=display_file_name(path_or_name)
  pattern_names[p]=(n~="" and n or nil)
  if reaper.SetProjExtState then reaper.SetProjExtState(0,PROJECT_EXT_SECTION,string.format("pattern_name_%03d",p),n) end
end
function set_song_name(i,path_or_name)
  i=clampi(i or current_song,0,MAX_SONGS-1)
  local n=display_file_name(path_or_name):gsub("[\r\n\t]"," "):gsub("^%s+",""):gsub("%s+$","")
  song_names[i]=(n~="" and n or nil)
  if songs and songs[i] then songs[i].name=(n~="" and n or nil) end
  if reaper.SetProjExtState then reaper.SetProjExtState(0,PROJECT_EXT_SECTION,string.format("song_name_%02d",i),n) end
end
function load_project_display_names()
  if not reaper.GetProjExtState then return end
  for p=0,MAX_PATTERNS-1 do
    local _,n=reaper.GetProjExtState(0,PROJECT_EXT_SECTION,string.format("pattern_name_%03d",p))
    if n and n~="" then pattern_names[p]=n end
  end
  for i=0,MAX_SONGS-1 do
    local _,n=reaper.GetProjExtState(0,PROJECT_EXT_SECTION,string.format("song_name_%02d",i))
    if n and n~="" then song_names[i]=n end
  end
end
function song_session_hash()
  local s=norm_path(song_root_dir()):lower()
  local h=2166136261
  for i=1,#s do h=((h ~ s:byte(i))*16777619) & 0xFFFFFFFF end
  return string.format("%08X",h)
end
function session_pattern_key(p)
  return string.format("pattern_ram_%s_%03d",song_session_hash(),clampi(p or current_pattern,0,MAX_PATTERNS-1))
end
function set_step_recording_midi_isolation(active)
  reaper.gmem_attach(GMEM_NS)
  if active then
    -- During step recording the keyboard must not trigger assigned patterns.
    -- Clearing the map also lets the same keys reach the synth normally.
    for n=0,127 do reaper.gmem_write(MIDI_MAP_BASE+n,0) end
  else
    rebuild_midi_pattern_bank()
  end
end

function biscot_serialize_pattern(p)
  p=clampi(p or current_pattern,0,MAX_PATTERNS-1)
  local pat=ensure_pattern(p)
  local lines={"BISCOT1_TRACKER_RAM_V2","pattern="..p,"length="..clampi(pat.length or 16,1,MAX_STEPS),"repeat="..clampi(pat.repeat_count or 1,1,1000),"midi_note="..clampi(pat.midi_note or -1,-1,127),"release_arp="..clampi(pat.release_arp or 0,0,1)}
  local rc=clampi(pat.repeat_count or 1,1,1000)
  for i=1,rc do
    lines[#lines+1]="speed\t"..i.."\t"..tostring(clamp(tonumber(pat.repeat_speeds[i]) or 1,0.25,16))
    lines[#lines+1]="transpose\t"..i.."\t"..tostring(clampi(pat.repeat_transposes[i] or 0,-96,96))
  end
  for st=0,MAX_STEPS-1 do
    for tab=1,3 do
      local cells=pat.steps[st][tab] or {}
      if #cells>0 then
        lines[#lines+1]="step\t"..st.."\t"..tab
        for _,c in ipairs(cells) do lines[#lines+1]="cell\t"..tostring(c.id).."\t"..tostring(c.value) end
      end
    end
  end
  return table.concat(lines,"\n")
end
function save_ram_pattern_snapshot(p,mark_newer_than_composition)
  if not reaper.SetExtState then return end
  p=clampi(p or current_pattern,0,MAX_PATTERNS-1)
  local data=biscot_serialize_pattern(p)
  reaper.SetExtState(EXT_SECTION,session_pattern_key(p),data,false)
  if reaper.SetProjExtState then
    reaper.SetProjExtState(0,PROJECT_EXT_SECTION,string.format("pattern_%03d",p),data)
    if p==current_pattern then
      reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"active_pattern",tostring(p))
      reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"active_pattern_data",data)
    end
    if mark_newer_than_composition then
      -- Pattern edits belong to the RPP even when no Pattern TXT or Composition
      -- is exported.  The Startup Loader overlays these indexed snapshots after
      -- composition_raw, exactly as it already does for edited Songs.
      reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"pattern_slots_override_composition","1")
      if reaper.MarkProjectDirty then reaper.MarkProjectDirty(0) end
    end
  end
  ram_pattern_loaded[p]=true
end
function load_ram_pattern_snapshot(p)
  if not reaper.GetExtState then return false end
  p=clampi(p or current_pattern,0,MAX_PATTERNS-1)
  local txt=reaper.GetExtState(EXT_SECTION,session_pattern_key(p)) or ""
  if not txt:find("BISCOT1_TRACKER_RAM_V2",1,true) then return false end
  local pat={length=16,speed=1,repeat_count=1,repeat_speeds={1},repeat_transposes={0},midi_note=-1,release_arp=0,steps={}}
  for st=0,MAX_STEPS-1 do pat.steps[st]={{},{},{}} end
  local curst,curtab=nil,nil
  for line in (txt.."\n"):gmatch("([^\n]*)\n") do
    if line:match("^length=") then pat.length=clampi(tonumber(line:match("^length=(%-?%d+)")) or 16,1,MAX_STEPS)
    elseif line:match("^repeat=") then pat.repeat_count=clampi(tonumber(line:match("^repeat=(%-?%d+)")) or 1,1,1000)
    elseif line:match("^midi_note=") then pat.midi_note=clampi(tonumber(line:match("^midi_note=(%-?%d+)")) or -1,-1,127)
    elseif line:match("^release_arp=") then pat.release_arp=clampi(tonumber(line:match("^release_arp=(%-?%d+)")) or 0,0,1)
    elseif line:match("^speed\t") then
      local i,v=line:match("^speed\t([^\t]+)\t([^\t]+)"); i=tonumber(i); v=tonumber(v)
      if i then pat.repeat_speeds[i]=clamp(v or 1,0.25,16) end
    elseif line:match("^transpose\t") then
      local i,v=line:match("^transpose\t([^\t]+)\t([^\t]+)"); i=tonumber(i); v=tonumber(v)
      if i then pat.repeat_transposes[i]=clampi(v or 0,-96,96) end
    elseif line:match("^step\t") then
      local a,b=line:match("^step\t([^\t]+)\t([^\t]+)")
      curst=clampi(tonumber(a) or 0,0,MAX_STEPS-1); curtab=clampi(tonumber(b) or 1,1,3)
    elseif line:match("^cell\t") and curst~=nil and curtab then
      local a,b=line:match("^cell\t([^\t]+)\t([^\t]+)")
      if a then table.insert(pat.steps[curst][curtab],{id=tonumber(a) or 0,value=tonumber(b) or 0}) end
    end
  end
  for i=1,pat.repeat_count do
    if pat.repeat_speeds[i]==nil then pat.repeat_speeds[i]=1 end
    if pat.repeat_transposes[i]==nil then pat.repeat_transposes[i]=0 end
  end
  patterns[p]=pat
  ram_pattern_loaded[p]=true
  return true
end
function load_project_pattern_snapshot()
  if not reaper.GetProjExtState then return false end
  local _,ps=reaper.GetProjExtState(0,PROJECT_EXT_SECTION,"active_pattern")
  local p=tonumber(ps)
  local _,txt=reaper.GetProjExtState(0,PROJECT_EXT_SECTION,"active_pattern_data")
  if p==nil or not txt or not txt:find("BISCOT1_TRACKER_RAM_V2",1,true) then return false end
  p=clampi(p,0,MAX_PATTERNS-1)
  local pat={length=16,speed=1,repeat_count=1,repeat_speeds={1},repeat_transposes={0},midi_note=-1,release_arp=0,steps={}}
  for st=0,MAX_STEPS-1 do pat.steps[st]={{},{},{}} end
  local curst,curtab=nil,nil
  for line in (txt.."\n"):gmatch("([^\n]*)\n") do
    if line:match("^length=") then pat.length=clampi(tonumber(line:match("^length=(%-?%d+)")) or 16,1,MAX_STEPS)
    elseif line:match("^repeat=") then pat.repeat_count=clampi(tonumber(line:match("^repeat=(%-?%d+)")) or 1,1,1000)
    elseif line:match("^midi_note=") then pat.midi_note=clampi(tonumber(line:match("^midi_note=(%-?%d+)")) or -1,-1,127)
    elseif line:match("^release_arp=") then pat.release_arp=clampi(tonumber(line:match("^release_arp=(%-?%d+)")) or 0,0,1)
    elseif line:match("^speed\t") then
      local i,v=line:match("^speed\t([^\t]+)\t([^\t]+)"); i=tonumber(i); v=tonumber(v)
      if i then pat.repeat_speeds[i]=clamp(v or 1,0.25,16) end
    elseif line:match("^transpose\t") then
      local i,v=line:match("^transpose\t([^\t]+)\t([^\t]+)"); i=tonumber(i); v=tonumber(v)
      if i then pat.repeat_transposes[i]=clampi(v or 0,-96,96) end
    elseif line:match("^step\t") then
      local a,b=line:match("^step\t([^\t]+)\t([^\t]+)")
      curst=clampi(tonumber(a) or 0,0,MAX_STEPS-1); curtab=clampi(tonumber(b) or 1,1,3)
    elseif line:match("^cell\t") and curst~=nil and curtab then
      local a,b=line:match("^cell\t([^\t]+)\t([^\t]+)")
      if a then table.insert(pat.steps[curst][curtab],{id=tonumber(a) or 0,value=tonumber(b) or 0}) end
    end
  end
  for i=1,pat.repeat_count do
    if pat.repeat_speeds[i]==nil then pat.repeat_speeds[i]=1 end
    if pat.repeat_transposes[i]==nil then pat.repeat_transposes[i]=0 end
  end
  current_pattern=p
  patterns[p]=pat
  ram_pattern_loaded[p]=true
  return true
end

function load_all_ram_pattern_snapshots()
  for p=0,MAX_PATTERNS-1 do load_ram_pattern_snapshot(p) end
end
function load_all_project_pattern_snapshots()
  if not reaper.GetProjExtState then return end
  for p=0,MAX_PATTERNS-1 do
    local _,txt=reaper.GetProjExtState(0,PROJECT_EXT_SECTION,string.format("pattern_%03d",p))
    if txt and txt:find("BISCOT1_TRACKER_RAM_V2",1,true) then
      local pat={length=16,speed=1,repeat_count=1,repeat_speeds={1},repeat_transposes={0},midi_note=-1,release_arp=0,steps={}}
      for st=0,MAX_STEPS-1 do pat.steps[st]={{},{},{}} end
      local curst,curtab=nil,nil
      for line in (txt.."\n"):gmatch("([^\n]*)\n") do
        if line:match("^length=") then pat.length=clampi(tonumber(line:match("^length=(%-?%d+)")) or 16,1,MAX_STEPS)
        elseif line:match("^repeat=") then pat.repeat_count=clampi(tonumber(line:match("^repeat=(%-?%d+)")) or 1,1,1000)
        elseif line:match("^midi_note=") then pat.midi_note=clampi(tonumber(line:match("^midi_note=(%-?%d+)")) or -1,-1,127)
    elseif line:match("^release_arp=") then pat.release_arp=clampi(tonumber(line:match("^release_arp=(%-?%d+)")) or 0,0,1)
        elseif line:match("^speed\t") then local i,v=line:match("^speed\t([^\t]+)\t([^\t]+)"); i=tonumber(i); if i then pat.repeat_speeds[i]=clamp(tonumber(v) or 1,0.25,16) end
        elseif line:match("^transpose\t") then local i,v=line:match("^transpose\t([^\t]+)\t([^\t]+)"); i=tonumber(i); if i then pat.repeat_transposes[i]=clampi(tonumber(v) or 0,-96,96) end
        elseif line:match("^step\t") then local a,b=line:match("^step\t([^\t]+)\t([^\t]+)"); curst=clampi(tonumber(a) or 0,0,MAX_STEPS-1); curtab=clampi(tonumber(b) or 1,1,3)
        elseif line:match("^cell\t") and curst~=nil and curtab then local a,b=line:match("^cell\t([^\t]+)\t([^\t]+)"); if a then table.insert(pat.steps[curst][curtab],{id=tonumber(a) or 0,value=tonumber(b) or 0}) end end
      end
      for i=1,pat.repeat_count do
    if pat.repeat_speeds[i]==nil then pat.repeat_speeds[i]=1 end
    if pat.repeat_transposes[i]==nil then pat.repeat_transposes[i]=0 end
  end
      patterns[p]=pat; ram_pattern_loaded[p]=true
    end
  end
end

function activate_pattern(target)
  target=clampi(target,0,MAX_PATTERNS-1)
  if target==current_pattern then return false end
  save_ram_pattern_snapshot(current_pattern)
  current_pattern=target
  if not ram_pattern_loaded[target] then
    if not load_ram_pattern_snapshot(target) then
      ram_pattern_loaded[target]=true
      ensure_pattern(target)
    end
  else
    ensure_pattern(target)
  end
  return true
end

function load_patterns_from_dir(dir,clear_first)
  dir=norm_path(dir); if dir=="" then return 0 end
  if clear_first then patterns={} end
  local loaded=0; local i=0
  while true do
    local fn=reaper.EnumerateFiles(dir,i); if not fn then break end
    if tostring(fn):lower():match("%.txt$") then
      local path=join_path(dir,fn); local n=pattern_number_from_txt(path)
      if n and load_pattern_txt(path,n) then
        set_pattern_name(n,path)
        loaded=loaded+1
      end
    end
    i=i+1
  end
  return loaded
end
function load_all_patterns()
  ensure_dir(patterns_dir())
  load_patterns_from_dir(patterns_dir(),false)
  local a=tonumber((read_text_file(active_pattern_path()) or ""):match("%-?%d+"))
  if a then current_pattern=clampi(a,0,MAX_PATTERNS-1) end
end
function copy_loaded_session_to_container()
  ensure_dir(patterns_dir()); ensure_dir(songs_dir())
  local keep=current_pattern
  for p=0,MAX_PATTERNS-1 do
    local pat=patterns[p]
    if pat and pattern_has_content(pat) then write_text_file(pattern_path(p),biscot_serialize_pattern(p)) end
  end
  write_text_file(active_pattern_path(),tostring(keep))
  save_song_project()
end

function path_exists(path)
  if path=="" then return false end
  local f=io.open(path,"rb")
  if f then f:close(); return true end
  local ok=os.rename(path,path)
  return ok==true
end
function is_directory(path)
  path=norm_path(path)
  if path=="" then return false end
  local test=reaper.EnumerateSubdirectories(path,0)
  if test~=nil then return true end
  local ftest=reaper.EnumerateFiles(path,0)
  if ftest~=nil then return true end
  return path==norm_path(tracker_root_dir()) or path==norm_path(song_root_dir()) or path==norm_path(patterns_dir()) or path==norm_path(songs_dir()) or path==norm_path(library_root_dir()) or path==norm_path(library_patterns_dir()) or path==norm_path(library_songs_dir()) or path==norm_path(library_compositions_dir())
end
local f5_virtual_root=true
local f5_scope_root=""
local f5_scope_label=""
function f5_project_mode()
  return project_tracker_root_dir()~=""
end
function f5_root_dir()
  return norm_path(tracker_root_dir())
end
function f5_clamp_path(path)
  path=norm_path(path)
  local root=norm_path(f5_scope_root~="" and f5_scope_root or f5_root_dir())
  if path=="" or path:sub(1,#root)~=root then return root end
  return path
end
function f5_entry_label(e)
  if not e then return "empty" end
  return e.name..","
end
function refresh_f5_entries(keep_name)
  f5_entries={}
  if f5_virtual_root then
    local lib=norm_path(library_root_dir())
    if lib~="" then f5_entries[#f5_entries+1]={name="Library",path=lib,kind="folder",virtual_library=true} end
    local project_root=norm_path(project_tracker_root_dir())
    if project_root~="" then f5_entries[#f5_entries+1]={name="Project",path=project_root,kind="folder",virtual_project=true} end
    local default_root=norm_path(default_tracker_root_dir())
    f5_entries[#f5_entries+1]={name="Default",path=default_root,kind="folder",virtual_default=true}
  else
    f5_path=f5_clamp_path(f5_path~="" and f5_path or f5_root_dir())
    local i=0
    while true do
      local dn=reaper.EnumerateSubdirectories(f5_path,i); if not dn then break end
      local hide_legacy_banks=(norm_path(f5_path)==norm_path(library_root_dir()) and tostring(dn):lower()=="banks")
      if not hide_legacy_banks then
        f5_entries[#f5_entries+1]={name=tostring(dn),path=join_path(f5_path,dn),kind="folder"}
      end
      i=i+1
    end
    i=0
    while true do
      local fn=reaper.EnumerateFiles(f5_path,i); if not fn then break end
      local path=join_path(f5_path,fn)
      local txt_kind=nil
      if tostring(fn):lower():match("%.txt$") then txt_kind=f5_txt_kind(path) end
      f5_entries[#f5_entries+1]={
        name=tostring(fn),path=path,kind="file",
        txt_kind=txt_kind,pattern=(txt_kind=="pattern") and pattern_number_from_txt(path) or nil
      }
      i=i+1
    end
    table.sort(f5_entries,function(a,b)
      if a.kind~=b.kind then return a.kind=="folder" end
      return a.name:lower()<b.name:lower()
    end)
  end
  if #f5_entries==0 then f5_index=1 else
    local found=nil
    if keep_name then for j,e in ipairs(f5_entries) do if e.name==keep_name then found=j break end end end
    f5_index=clampi(found or f5_index or 1,1,#f5_entries)
  end
end
function f5_speak()
  local e=f5_entries[f5_index]
  if e then speak(f5_entry_label(e)) else speak("empty") end
end
function f5_enter_folder(e)
  if not e or e.kind~="folder" then return false end
  if f5_virtual_root then
    f5_virtual_root=false
    f5_scope_label=e.name or ""
    if e.virtual_library then f5_scope_root=norm_path(library_root_dir())
    elseif e.virtual_project then f5_scope_root=norm_path(project_tracker_root_dir())
    else f5_scope_root=norm_path(default_tracker_root_dir()) end
  end
  f5_path=f5_clamp_path(e.path); f5_index=1; refresh_f5_entries(); update_controller_title(); f5_speak(); return true
end
function f5_go_parent()
  if f5_virtual_root then return true end
  local root=norm_path(f5_scope_root~="" and f5_scope_root or f5_root_dir())
  local cur=norm_path(f5_path)
  local old=base_name(cur)
  if cur==root then
    local return_label=f5_scope_label
    f5_virtual_root=true; f5_scope_root=""; f5_scope_label=""; f5_path=""; f5_index=1
    refresh_f5_entries(return_label); update_controller_title(); f5_speak(); return true
  end
  local p=parent_dir(cur)
  if p=="" or p:sub(1,#root)~=root then p=root end
  f5_path=p; f5_index=1; refresh_f5_entries(old); update_controller_title(); f5_speak(); return true
end
function sanitize_name(name,is_file,file_prefix)
  name=tostring(name or ""):gsub('[\\/:*?"<>|]',"-"):gsub("^%s+",""):gsub("%s+$","")
  if is_file then
    name=name:gsub("%.txt$","")
    if file_prefix and file_prefix~="" then
      local low=name:lower()
      local prefix_low=file_prefix:lower().."-"
      if low:sub(1,#prefix_low)~=prefix_low then name=file_prefix.."-"..name end
    end
    if name~="" then name=name..".txt" end
  end
  return name
end
function rename_f5_current()
  local e=f5_entries[f5_index]
  if not e then speak("Nothing to rename"); return true end
  if e.virtual_library or e.virtual_project or e.virtual_default then
    speak("Rename unavailable here"); return true
  end
  local can_rename=false
  if e.kind=="file" then
    can_rename=(e.txt_kind=="pattern" or e.txt_kind=="song" or f5_txt_kind(e.path)=="pattern" or f5_txt_kind(e.path)=="song")
  elseif e.kind=="folder" then
    local low=tostring(e.name or ""):lower()
    can_rename=low~="banks" and low~="patterns" and low~="pattern" and low~="song" and low~="songs"
  end
  if not can_rename then speak("Rename unavailable here"); return true end
  local ok,new_name=reaper.GetUserInputs("Rename",1,"New name",e.name)
  -- GetUserInputs transfers keyboard focus back to REAPER.  Restore the
  -- controller explicitly so a second F2 is still handled by the F5 browser
  -- instead of reaching the TCP.
  if restore_controller_focus_after_dialog then restore_controller_focus_after_dialog() end
  if not ok then speak("Cancelled"); return true end
  new_name=sanitize_name(new_name,e.kind=="file",nil)
  if new_name=="" then speak("Invalid name"); return true end
  if new_name==e.name then speak("Name unchanged"); return true end
  local new_path=join_path(f5_path,new_name)
  if path_exists(new_path) then speak("Name already exists"); return true end
  local old_path=e.path
  local renamed=os.rename(old_path,new_path)
  if not renamed then speak("Rename failed"); return true end
  local kind=e.txt_kind or f5_txt_kind(new_path)
  if kind=="pattern" then
    local p=pattern_number_from_txt(new_path)
    if p~=nil then set_pattern_name(p,new_path) end
    local active=norm_path(read_text_file(active_pattern_file_path()) or "")
    if active==norm_path(old_path) then write_text_file(active_pattern_file_path(),norm_path(new_path)) end
  elseif kind=="song" then
    if song_names[current_song]==display_file_name(old_path) then set_song_name(current_song,new_path) end
    local active=norm_path(read_text_file(active_song_file_path()) or "")
    if active==norm_path(old_path) then write_text_file(active_song_file_path(),norm_path(new_path)) end
  end
  local root=norm_path(f5_scope_root~="" and f5_scope_root or f5_root_dir())
  if e.kind=="folder" and parent_dir(old_path)==root then
    local active=read_marker(root)
    if active==e.name then write_text_file(active_song_marker_path(root),new_name) end
  end
  refresh_f5_entries(new_name)
  -- Let the screen reader finish announcing the controller window title after
  -- focus returns.  Speaking immediately here can be swallowed by NVDA/OSARA.
  local rename_message="Renamed "..new_name
  local rename_announce_at=(reaper.time_precise and reaper.time_precise() or 0)+0.38
  local function announce_rename_after_title()
    local now=reaper.time_precise and reaper.time_precise() or rename_announce_at
    if now<rename_announce_at then
      reaper.defer(announce_rename_after_title)
    elseif page==5 then
      speak(rename_message)
    end
  end
  reaper.defer(announce_rename_after_title)
  return true
end
function biscot_parse_pattern_snapshot(txt,p)
  if not txt or not txt:find("BISCOT1_TRACKER_RAM_V2",1,true) then return nil end
  p=clampi(p or tonumber(txt:match("pattern=(%-?%d+)")) or current_pattern,0,MAX_PATTERNS-1)
  local pat={length=16,speed=1,repeat_count=1,repeat_speeds={1},repeat_transposes={0},midi_note=-1,release_arp=0,steps={}}
  for st=0,MAX_STEPS-1 do pat.steps[st]={{},{},{}} end
  local curst,curtab=nil,nil
  for line in (txt.."\n"):gmatch("([^\n]*)\n") do
    if line:match("^length=") then pat.length=clampi(tonumber(line:match("^length=(%-?%d+)")) or 16,1,MAX_STEPS)
    elseif line:match("^repeat=") then pat.repeat_count=clampi(tonumber(line:match("^repeat=(%-?%d+)")) or 1,1,1000)
    elseif line:match("^midi_note=") then pat.midi_note=clampi(tonumber(line:match("^midi_note=(%-?%d+)")) or -1,-1,127)
    elseif line:match("^release_arp=") then pat.release_arp=clampi(tonumber(line:match("^release_arp=(%-?%d+)")) or 0,0,1)
    elseif line:match("^speed\t") then
      local i,v=line:match("^speed\t([^\t]+)\t([^\t]+)"); i=tonumber(i)
      if i then pat.repeat_speeds[i]=clamp(tonumber(v) or 1,0.25,16) end
    elseif line:match("^transpose\t") then
      local i,v=line:match("^transpose\t([^\t]+)\t([^\t]+)"); i=tonumber(i)
      if i then pat.repeat_transposes[i]=clampi(tonumber(v) or 0,-96,96) end
    elseif line:match("^step\t") then
      local a,b=line:match("^step\t([^\t]+)\t([^\t]+)")
      curst=clampi(tonumber(a) or 0,0,MAX_STEPS-1); curtab=clampi(tonumber(b) or 1,1,3)
    elseif line:match("^cell\t") and curst~=nil and curtab then
      local a,b=line:match("^cell\t([^\t]+)\t([^\t]+)")
      if a then table.insert(pat.steps[curst][curtab],{id=tonumber(a) or 0,value=tonumber(b) or 0}) end
    end
  end
  for i=1,pat.repeat_count do
    if pat.repeat_speeds[i]==nil then pat.repeat_speeds[i]=1 end
    if pat.repeat_transposes[i]==nil then pat.repeat_transposes[i]=0 end
  end
  return pat
end

function biscot_prepare_pattern_undo_baseline()
  local p=clampi(current_pattern,0,MAX_PATTERNS-1)
  if pattern_undo_baseline[p]==nil then pattern_undo_baseline[p]=biscot_serialize_pattern(p) end
end

function biscot_prepare_song_undo_baseline()
  local i=clampi(current_song,0,MAX_SONGS-1)
  if song_undo_baseline[i]==nil then song_undo_baseline[i]=serialize_song(song) end
end

function biscot_record_pattern_undo_after_edit()
  if undo_restoring then return end
  local p=clampi(current_pattern,0,MAX_PATTERNS-1)
  local now=biscot_serialize_pattern(p)
  local before=pattern_undo_baseline[p]
  if before==nil then pattern_undo_baseline[p]=now; return end
  if now==before then return end
  local stack=pattern_undo_stacks[p] or {}
  stack[#stack+1]=before
  if #stack>UNDO_LIMIT then table.remove(stack,1) end
  pattern_undo_stacks[p]=stack
  pattern_undo_baseline[p]=now
end

function biscot_record_song_undo_after_edit()
  if undo_restoring then return end
  local i=clampi(current_song,0,MAX_SONGS-1)
  local now=serialize_song(song)
  local before=song_undo_baseline[i]
  if before==nil then song_undo_baseline[i]=now; return end
  if now==before then return end
  local stack=song_undo_stacks[i] or {}
  stack[#stack+1]=before
  if #stack>UNDO_LIMIT then table.remove(stack,1) end
  song_undo_stacks[i]=stack
  song_undo_baseline[i]=now
end

function biscot_undo_pattern()
  local p=clampi(current_pattern,0,MAX_PATTERNS-1)
  local stack=pattern_undo_stacks[p]
  if not stack or #stack==0 then speak("Nothing to undo") return true end
  local txt=table.remove(stack)
  local restored=biscot_parse_pattern_snapshot(txt,p)
  if not restored then speak("Undo failed") return true end
  undo_restoring=true
  patterns[p]=restored
  pattern_undo_baseline[p]=txt
  current_step=clampi(current_step,-1,pattern_length()+repeat_speed_row_count())
  if is_real_step() then
    local cells=current_cells()
    logical_cell=clampi(logical_cell,1,math.max(1,#cells)); current_cell=logical_cell
  else
    current_cell=clampi(current_cell,1,math.max(1,meta_cell_count()))
  end
  clear_pattern_step_selection()
  refresh_pattern_after_edit()
  undo_restoring=false
  speak("Undo pattern")
  return true
end

function biscot_undo_song()
  local i=clampi(current_song,0,MAX_SONGS-1)
  local stack=song_undo_stacks[i]
  if not stack or #stack==0 then speak("Nothing to undo") return true end
  local txt=table.remove(stack)
  local restored=parse_song_text(txt)
  if not restored then speak("Undo failed") return true end
  undo_restoring=true
  songs[i]=restored; song=restored
  load_song_mixer_monitor_state(song)
  song_undo_baseline[i]=txt
  song_step=clampi(song_step,-1,song_lane_length(song_lane))
  song_cell=math.max(1,song_cell)
  clear_song_step_selection()
  refresh_song_after_edit()
  undo_restoring=false
  speak("Undo song")
  return true
end

refresh_pattern_after_edit=function()
  biscot_record_pattern_undo_after_edit()
  -- Editing while the pattern is running must never restart the performance.
  -- Repeated key presses previously incremented the run serial and accumulated
  -- complete pattern restarts in the core queue. Update RAM only; the tracker
  -- reads the new values on the next unscheduled step/loop.
  write_pattern_memory()
  if autosave_enabled then save_pattern_txt(current_pattern) end
  save_ram_pattern_snapshot(nil,true)
  rebuild_midi_pattern_bank()
  save_session_state()
end

function norm(s) return tostring(s or ""):lower():gsub("\\","/") end
function find_fx(track,kind)
  for i=0,reaper.TrackFX_GetCount(track)-1 do
    local _,n=reaper.TrackFX_GetFXName(track,i,"")
    n=norm(n)
    if kind=="tracker" then
      if n:find("biscot1",1,true) and n:find("tracker",1,true) then return i end
    else
      if n:find("biscot1",1,true) and not n:find("tracker",1,true) then return i end
    end
  end
  return -1
end
function get_or_create_work_track()
  local tr=reaper.GetSelectedTrack(0,0)
  if not tr then
    if reaper.CountTracks(0)==0 then reaper.InsertTrackAtIndex(0,true) end
    tr=reaper.GetTrack(0,0)
  end
  return tr
end

function required_fx_state()
  local tr=get_or_create_work_track()
  if not tr then return nil,-1,-1 end
  return tr,find_fx(tr,"tracker"),find_fx(tr,"core")
end

-- For sample-near Song/Pattern launch the Tracker must process MIDI and
-- automation before the Core renders the block.  Reorder only these two FX;
-- every other FX keeps its relative position.
function ensure_tracker_before_core(tr)
  if not tr then return -1,-1,false end
  local ti=find_fx(tr,"tracker")
  local ci=find_fx(tr,"core")
  local moved=false
  if ti>=0 and ci>=0 and ti>ci and reaper.TrackFX_CopyToTrack then
    reaper.TrackFX_CopyToTrack(tr,ti,tr,ci,true)
    ti=find_fx(tr,"tracker")
    ci=find_fx(tr,"core")
    moved=ti>=0 and ci>=0 and ti<ci
  end
  return ti,ci,moved
end

function load_required_fx(tr,ti,ci)
  if not tr then return false end
  if ti<0 then
    ti=reaper.TrackFX_AddByName(tr,TRACKER_FX_PATH,false,-1000)
    if ti<0 then ti=reaper.TrackFX_AddByName(tr,TRACKER_FX_PATH:gsub("^JS:%s*",""),false,-1000) end
  end
  if ci<0 then
    ci=reaper.TrackFX_AddByName(tr,CORE_FX_PATH,false,-1000)
    if ci<0 then ci=reaper.TrackFX_AddByName(tr,CORE_FX_PATH:gsub("^JS:%s*",""),false,-1000) end
  end
  ti,ci=ensure_tracker_before_core(tr)
  return ti>=0 and ci>=0 and ti<ci
end

function find_core_track_and_fx()
  local selected=reaper.GetSelectedTrack(0,0)
  if selected then
    local ci=find_fx(selected,"core")
    if ci>=0 then return selected,ci end
  end
  for i=0,reaper.CountTracks(0)-1 do
    local tr=reaper.GetTrack(0,i)
    local ci=find_fx(tr,"core")
    if ci>=0 then return tr,ci end
  end
  return nil,-1
end


local BISCOT_OUTPUT_SOURCE_KEY="BISCOT1_OUTPUT_SOURCE"
local BISCOT_OUTPUT_ROLE_KEY="BISCOT1_OUTPUT_ROLE"

function biscot_track_ext_get(tr,key)
  if not tr or not reaper.GetSetMediaTrackInfo_String then return "" end
  local ok,value=reaper.GetSetMediaTrackInfo_String(tr,"P_EXT:"..key,"",false)
  return ok and tostring(value or "") or ""
end
function biscot_track_ext_set(tr,key,value)
  if tr and reaper.GetSetMediaTrackInfo_String then
    reaper.GetSetMediaTrackInfo_String(tr,"P_EXT:"..key,tostring(value or ""),true)
  end
end
function biscot_set_track_name(tr,name)
  if tr and reaper.GetSetMediaTrackInfo_String then
    reaper.GetSetMediaTrackInfo_String(tr,"P_NAME",tostring(name or ""),true)
  end
end
function biscot_output_tracks_for_source(source_guid)
  local roles={}
  for i=0,reaper.CountTracks(0)-1 do
    local tr=reaper.GetTrack(0,i)
    if biscot_track_ext_get(tr,BISCOT_OUTPUT_SOURCE_KEY)==source_guid then
      local role=biscot_track_ext_get(tr,BISCOT_OUTPUT_ROLE_KEY)
      if role~="" then roles[role]=tr end
    end
  end
  return roles
end
function biscot_tag_output_track(tr,source_guid,role,name)
  biscot_track_ext_set(tr,BISCOT_OUTPUT_SOURCE_KEY,source_guid)
  biscot_track_ext_set(tr,BISCOT_OUTPUT_ROLE_KEY,role)
  biscot_set_track_name(tr,name)
end
function biscot_create_output_tracks(source,source_guid)
  if not source or not reaper.InsertTrackAtIndex then return nil end
  local source_index=math.max(0,math.floor(reaper.GetMediaTrackInfo_Value(source,"IP_TRACKNUMBER") or 1)-1)
  local insert_at=source_index+1
  -- When the Core track is itself a folder parent, place the output folder
  -- after its complete subtree. This avoids routing the stems back into the
  -- same track that generated them.
  local folder_depth=math.floor(reaper.GetMediaTrackInfo_Value(source,"I_FOLDERDEPTH") or 0)
  if folder_depth>0 then
    local depth=folder_depth
    local i=source_index+1
    local count=reaper.CountTracks(0)
    while i<count and depth>0 do
      local child=reaper.GetTrack(0,i)
      depth=depth+math.floor(reaper.GetMediaTrackInfo_Value(child,"I_FOLDERDEPTH") or 0)
      i=i+1
    end
    insert_at=i
  end
  for i=0,4 do reaper.InsertTrackAtIndex(insert_at+i,true) end
  local roles={
    folder=reaper.GetTrack(0,insert_at),
    op1=reaper.GetTrack(0,insert_at+1),
    op2=reaper.GetTrack(0,insert_at+2),
    op3=reaper.GetTrack(0,insert_at+3),
    filter=reaper.GetTrack(0,insert_at+4),
  }
  if not (roles.folder and roles.op1 and roles.op2 and roles.op3 and roles.filter) then return nil end
  biscot_tag_output_track(roles.folder,source_guid,"folder","Biscot1 Outputs")
  biscot_tag_output_track(roles.op1,source_guid,"op1","Biscot1 Op1")
  biscot_tag_output_track(roles.op2,source_guid,"op2","Biscot1 Op2")
  biscot_tag_output_track(roles.op3,source_guid,"op3","Biscot1 Op3")
  biscot_tag_output_track(roles.filter,source_guid,"filter","Biscot1 Filter")
  reaper.SetMediaTrackInfo_Value(roles.folder,"I_FOLDERDEPTH",1)
  reaper.SetMediaTrackInfo_Value(roles.op1,"I_FOLDERDEPTH",0)
  reaper.SetMediaTrackInfo_Value(roles.op2,"I_FOLDERDEPTH",0)
  reaper.SetMediaTrackInfo_Value(roles.op3,"I_FOLDERDEPTH",0)
  reaper.SetMediaTrackInfo_Value(roles.filter,"I_FOLDERDEPTH",-1)
  return roles
end
function biscot_ensure_stem_send(source,dest,source_channel)
  if not source or not dest or not reaper.CreateTrackSend then return false end
  local send_index=-1
  local count=reaper.GetTrackNumSends(source,0)
  for i=0,count-1 do
    if reaper.GetTrackSendInfo_Value(source,0,i,"P_DESTTRACK")==dest then send_index=i; break end
  end
  if send_index<0 then send_index=reaper.CreateTrackSend(source,dest) end
  if send_index<0 then return false end
  reaper.SetTrackSendInfo_Value(source,0,send_index,"I_SRCCHAN",source_channel)
  reaper.SetTrackSendInfo_Value(source,0,send_index,"I_DSTCHAN",0)
  reaper.SetTrackSendInfo_Value(source,0,send_index,"I_SENDMODE",3) -- post-FX, pre-fader
  reaper.SetTrackSendInfo_Value(source,0,send_index,"D_VOL",1.0)
  reaper.SetTrackSendInfo_Value(source,0,send_index,"B_MUTE",0)
  return true
end
function biscot_ensure_multichannel_outputs(source)
  if not source then return false end
  local source_guid=tostring(reaper.GetTrackGUID and reaper.GetTrackGUID(source) or "")
  if source_guid=="" then return false end
  local roles=biscot_output_tracks_for_source(source_guid)
  if not (roles.folder and roles.op1 and roles.op2 and roles.op3 and roles.filter) then
    roles=biscot_create_output_tracks(source,source_guid)
  end
  if not roles then return false end
  reaper.SetMediaTrackInfo_Value(source,"I_NCHAN",math.max(10,reaper.GetMediaTrackInfo_Value(source,"I_NCHAN") or 2))
  -- Explicitly map the ten Core output pins to track channels 1..10. This also
  -- repairs older instances that were inserted while the track had two channels.
  local core_fx=find_fx(source,"core")
  if core_fx>=0 and reaper.TrackFX_SetPinMappings then
    for pin=0,9 do reaper.TrackFX_SetPinMappings(source,core_fx,1,pin,math.floor(2^pin),0) end
  end
  for _,role in ipairs({"folder","op1","op2","op3","filter"}) do
    local tr=roles[role]
    if tr then reaper.SetMediaTrackInfo_Value(tr,"I_NCHAN",2); reaper.SetMediaTrackInfo_Value(tr,"B_MUTE",0) end
  end
  biscot_ensure_stem_send(source,roles.op1,2)   -- Core outputs 3/4
  biscot_ensure_stem_send(source,roles.op2,4)   -- Core outputs 5/6
  biscot_ensure_stem_send(source,roles.op3,6)   -- Core outputs 7/8
  biscot_ensure_stem_send(source,roles.filter,8) -- Core outputs 9/10
  return true
end
function toggle_core_model()
  local tr,ci=find_core_track_and_fx()
  if not tr or ci<0 then speak("Biscot1 Core not found"); return false end
  local current=reaper.TrackFX_GetParam(tr,ci,4) or 0 -- slider5, zero-based index 4
  local value=current<0.5 and 1 or 0
  reaper.TrackFX_SetParam(tr,ci,4,value)
  speak(value>=0.5 and "Model, 6581 Dirtier" or "Model, 8580 Clearer")
  return true
end
function toggle_core_audio_mode()
  local tr,ci=find_core_track_and_fx()
  if not tr or ci<0 then speak("Biscot1 Core not found"); return false end
  local current=reaper.TrackFX_GetParam(tr,ci,1) or 0 -- slider2, zero-based index 1
  local value=current<0.5 and 1 or 0
  if reaper.Undo_BeginBlock2 then reaper.Undo_BeginBlock2(0) elseif reaper.Undo_BeginBlock then reaper.Undo_BeginBlock() end
  if reaper.PreventUIRefresh then reaper.PreventUIRefresh(1) end
  local outputs_ok=true
  if value>=0.5 then
    outputs_ok=biscot_ensure_multichannel_outputs(tr)
    reaper.SetMediaTrackInfo_Value(tr,"B_MAINSEND",0)
  else
    reaper.SetMediaTrackInfo_Value(tr,"B_MAINSEND",1)
  end
  reaper.TrackFX_SetParam(tr,ci,1,value)
  if reaper.PreventUIRefresh then reaper.PreventUIRefresh(-1) end
  if reaper.TrackList_AdjustWindows then reaper.TrackList_AdjustWindows(false) end
  if reaper.UpdateArrange then reaper.UpdateArrange() end
  local undo_name=value>=0.5 and "Biscot1 multichannel outputs" or "Biscot1 stereo mode"
  if reaper.Undo_EndBlock2 then reaper.Undo_EndBlock2(0,undo_name,-1) elseif reaper.Undo_EndBlock then reaper.Undo_EndBlock(undo_name,-1) end
  if value>=0.5 then
    speak(outputs_ok and "Audio Mode, Multichannel. Op1 3 4, Op2 5 6, Op3 7 8, Filter 9 10" or "Audio Mode, Multichannel. Output tracks not created")
  else
    speak("Audio Mode, Stereo")
  end
  return true
end

-- Keep Tracker before Core: GMEM carries the events, while this order also avoids a block of avoidable trigger latency.
function biscot_set_core_midi_mode_for_page(pg)
  local tr,ci=find_core_track_and_fx()
  if not tr or ci<0 then return false end
  -- The tracker package always uses the core in MIDI Channels 1-3 mode.
  -- F1/F5 trigger only channel/operator 1; F3 distributes the three lanes
  -- across channels/operators 1, 2 and 3. Slider 3 spans 0..2, therefore the
  -- normalized maximum 1.0 selects the third value deterministically.
  if reaper.TrackFX_SetParamNormalized then
    reaper.TrackFX_SetParamNormalized(tr,ci,2,1.0)
  else
    reaper.TrackFX_SetParam(tr,ci,2,2.0)
  end
  return true
end
function initialize_all_operator_levels()
  local tr,ci=find_core_track_and_fx()
  if not tr or ci<0 then return false end
  -- REAPER parameter indexes are zero-based:
  -- slider16, slider17 and slider18 are indexes 15, 16 and 17.
  reaper.TrackFX_SetParam(tr,ci,15,1.0)
  reaper.TrackFX_SetParam(tr,ci,16,1.0)
  reaper.TrackFX_SetParam(tr,ci,17,1.0)
  return true
end

function reset_core_to_defaults()
  local tr,ci=find_core_track_and_fx()
  if not tr or ci<0 then return false end
  for slider,value in pairs(CORE_SLIDER_DEFAULTS) do
    reaper.TrackFX_SetParam(tr,ci,slider-1,value)
  end
  reaper.TrackFX_SetParam(tr,ci,0,CORE_SLIDER_DEFAULTS[1] or 1)
  if reaper.TrackFX_SetParamNormalized then
    reaper.TrackFX_SetParamNormalized(tr,ci,2,1.0)
  else
    reaper.TrackFX_SetParam(tr,ci,2,2.0)
  end
  return true
end

function reset_current_parameter_to_default()
  local pat=ensure_pattern(current_pattern)
  if is_header_row() then
    if current_cell==1 then pat.length=16
    elseif current_cell==2 then pat.midi_note=-1; rebuild_midi_pattern_bank()
    elseif current_cell==3 then step_recording_active=false; step_record_sustain_down=false; set_step_recording_midi_isolation(false)
    else pat.release_arp=0; rebuild_midi_pattern_bank() end
  elseif is_repeat_row() then
    pat.repeat_count=1
    pat.repeat_speeds[1]=pat.repeat_speeds[1] or 1.0
    current_cell=1
  elseif is_speed_row() then
    local pair=math.floor((current_cell-1)/2)+1
    local ri=(speed_row_number()-1)*5+pair
    if ri>=1 and ri<=clampi(pat.repeat_count or 1,1,1000) then
      if current_cell%2==1 then pat.repeat_speeds[ri]=1.0 else pat.repeat_transposes[ri]=0 end
    end
  elseif is_real_step() then
    local c=current_cell_obj()
    local p=c and PARAM_BY_ID[c.id] or nil
    if not c or not p then return false end
    c.value=p.default
  else
    return false
  end
  refresh_pattern_after_edit()
  speak_cell()
  return true
end


------------------------------------------------------------
-- PROJECT STARTUP LOADER SHORTCUTS
-- Ctrl+Shift+C copies the Biscot1 startup loader named command ID.
-- Ctrl+L saves the current pattern, closes the controller and opens
-- SWS/S&M: Set project startup action.
------------------------------------------------------------
function biscot_trim(s)
  return (tostring(s or "")):gsub("^%s+", ""):gsub("%s+$", "")
end

function biscot_normalize_action_search_text(s)
  s=tostring(s or ""):lower()
  s=s:gsub("\\", "/"):gsub('"', ""):gsub("'", ""):gsub("%s+", " ")
  return s
end

function biscot_startup_loader_action_line_matches(text)
  local t=biscot_normalize_action_search_text(text)
  if t=="" then return false end
  if t:find("script: biscot1 tracker project startup loader.lua",1,true) then return true end
  if t:find("script: biscot1 tracker project startup loader",1,true) then return true end
  if t:find("biscot1 tracker project startup loader.lua",1,true) then return true end
  return t:find("biscot1",1,true) and t:find("tracker",1,true) and
         t:find("project",1,true) and t:find("startup",1,true) and t:find("loader",1,true)
end

function biscot_find_loader_id_from_action_list()
  if not (reaper.kbd_enumerateActions and reaper.kbd_getTextFromCmd and reaper.ReverseNamedCommandLookup) then return "" end
  local section=0
  for i=0,200000 do
    local ok,cmd=pcall(reaper.kbd_enumerateActions,section,i)
    if not ok or not cmd or cmd==0 then break end
    local ok_text,desc=pcall(reaper.kbd_getTextFromCmd,cmd,section)
    desc=ok_text and tostring(desc or "") or ""
    if biscot_startup_loader_action_line_matches(desc) then
      local named=reaper.ReverseNamedCommandLookup(cmd) or ""
      if named~="" then return named end
    end
  end
  return ""
end

function biscot_find_loader_id_from_kb_ini()
  local resource=reaper.GetResourcePath and reaper.GetResourcePath() or ""
  if resource=="" then return "" end
  local f=io.open(join_path(resource,"reaper-kb.ini"),"rb")
  if not f then return "" end
  local found=""
  for line in f:lines() do
    local l=tostring(line or "")
    if biscot_startup_loader_action_line_matches(l) then
      local id=l:match("(_?RS[%w_]+)")
      if id and id~="" then
        found=id
        local n=biscot_normalize_action_search_text(l)
        if n:find("biscot1 tracker project startup loader.lua",1,true) then break end
      end
    end
  end
  f:close()
  return found
end

function biscot_normalize_loader_command_id(id)
  id=biscot_trim(id):gsub("[\r\n\t ]+","")
  if id=="" then return "" end
  if id:match("^_RS[%w_]+$") then return id end
  if id:match("^RS[%w_]+$") then return "_"..id end
  local body=id:match("_?(RS[%w_]+)")
  if body and body~="" then return "_"..body end
  return id
end

function biscot_find_startup_loader_command_id()
  local id=biscot_normalize_loader_command_id(biscot_find_loader_id_from_action_list())
  if id~="" then return id end
  return biscot_normalize_loader_command_id(biscot_find_loader_id_from_kb_ini())
end

function biscot_copy_text_to_clipboard(text)
  text=tostring(text or "")
  if text=="" then return false end
  if reaper.CF_SetClipboard then reaper.CF_SetClipboard(text); return true end
  if gfx and gfx.setclipboard then gfx.setclipboard(text); return true end
  return false
end
function biscot_get_text_from_clipboard()
  if reaper.CF_GetClipboard then
    local ok,res=pcall(reaper.CF_GetClipboard, "")
    if ok and type(res)=="string" and res~="" then return res end
  end
  if gfx and gfx.getclipboard then
    local ok,res=pcall(gfx.getclipboard)
    if ok and type(res)=="string" and res~="" then return res end
  end
  return nil
end

function biscot_copy_loader_command_id()
  local id=biscot_find_startup_loader_command_id()
  if id=="" then speak("Startup loader action not found"); return true end
  if biscot_copy_text_to_clipboard(id) then
    speak("Startup loader command ID copied")
  else
    speak("Clipboard function not available")
  end
  return true
end

function biscot_sws_project_startup_action_command_id()
  if not reaper.NamedCommandLookup then return 0 end
  local candidates={"_S&M_SET_PRJ_ACTION","_S&M_PROJACTION","_SWS_PROJACTION","_S&M_SET_PROJACTION","_S&M_SET_PROJECT_STARTUP_ACTION"}
  for _,name in ipairs(candidates) do
    local cmd=reaper.NamedCommandLookup(name)
    if cmd and cmd~=0 then return cmd end
  end
  return 0
end

function biscot_open_sws_project_startup_action()
  local cmd=biscot_sws_project_startup_action_command_id()
  if not cmd or cmd==0 then speak("SWS project startup action not found"); return false end
  local ok=save_pattern_txt(current_pattern)
  if not ok then speak("Pattern save failed"); return false end
  save_session_state()
  if playing then playing=false; push_pattern(false) end
  gfx.quit()
  reaper.defer(function() reaper.Main_OnCommand(cmd,0) end)
  return true
end

function biscot_is_ctrl_shift_c(ch)
  local cap=gfx.mouse_cap or 0
  local is_ctrl=(cap&4)==4
  local is_shift=(cap&8)==8
  local is_alt=(cap&16)==16
  return is_ctrl and is_shift and not is_alt and (ch==3 or ch==string.byte("C") or ch==string.byte("c"))
end

function biscot_is_ctrl_l(ch)
  local cap=gfx.mouse_cap or 0
  local is_ctrl=(cap&4)==4
  local is_shift=(cap&8)==8
  local is_alt=(cap&16)==16
  return is_ctrl and not is_shift and not is_alt and (ch==12 or ch==string.byte("L") or ch==string.byte("l"))
end

local K={ESC=27,TAB=9,ENTER=13,SPACE=32,DELETE=6579564,BACKSPACE=8,BACKSPACE2=127,
  F1=26161,F2=26162,F3=26163,F4=26164,F5=26165,F6=26166,F12=6697266,
  UP=30064,DOWN=1685026670,LEFT=1818584692,RIGHT=1919379572,
  HOME=6647396,ENDK=1752132965,PAGE_UP=1885828464,PAGE_DOWN=1885824110,
  CTRL_C=3,CTRL_D=4,CTRL_J=10,CTRL_R=18,CTRL_S=19,CTRL_V=22,CTRL_X=24,CTRL_Z=26}
function alt() return ((gfx.mouse_cap or 0)&16)==16 end
function ctrl() return ((gfx.mouse_cap or 0)&4)==4 end
function shift() return ((gfx.mouse_cap or 0)&8)==8 end

function ensure_song_mixer(v)
  v=v or song
  v.mixer_volumes=v.mixer_volumes or {100,100,100}
  for i=1,3 do v.mixer_volumes[i]=clampi(v.mixer_volumes[i] or 100,0,100) end
  v.mixer_mute_mask=clampi(v.mixer_mute_mask or 0,0,7)
  v.mixer_solo_lane=clampi(v.mixer_solo_lane or 0,0,3)
  if v.mixer_solo_lane>0 then
    local bit=2^(v.mixer_solo_lane-1)
    if math.floor(v.mixer_mute_mask/bit)%2==1 then v.mixer_mute_mask=v.mixer_mute_mask-bit end
  end
  return v.mixer_volumes
end

function load_song_mixer_monitor_state(v)
  v=v or song
  ensure_song_mixer(v)
  local mask=clampi(v.mixer_mute_mask or 0,0,7)
  for i=1,3 do song_lane_muted[i]=(math.floor(mask/(2^(i-1)))%2)==1 end
  song_solo_lane=clampi(v.mixer_solo_lane or 0,0,3)
end

function save_song_mixer_monitor_state(v)
  v=v or song
  ensure_song_mixer(v)
  local mask=0
  for i=1,3 do if song_lane_muted[i] then mask=mask+(2^(i-1)) end end
  v.mixer_mute_mask=clampi(mask,0,7)
  v.mixer_solo_lane=clampi(song_solo_lane or 0,0,3)
  if v.mixer_solo_lane>0 then
    local bit=2^(v.mixer_solo_lane-1)
    if math.floor(v.mixer_mute_mask/bit)%2==1 then v.mixer_mute_mask=v.mixer_mute_mask-bit end
  end
end

-- Cells 3008-3010 are NOT free: they are lane-3 repeat speeds 998-1000.
-- The complete mixer is packed into the two genuinely free header cells.
function pack_song_mixer_header(v)
  v=v or song
  ensure_song_mixer(v)
  local v1=clampi(v.mixer_volumes[1] or 100,0,100)
  local v2=clampi(v.mixer_volumes[2] or 100,0,100)
  local v3=clampi(v.mixer_volumes[3] or 100,0,100)
  local volumes=1+v1+v2*101+v3*10201
  local state=SONG_MIXER_HEADER_MAGIC+clampi(v.mixer_mute_mask or 0,0,7)+clampi(v.mixer_solo_lane or 0,0,3)*8
  return volumes,state
end

function unpack_song_mixer_header(v,volumes_raw,state_raw)
  v=v or song
  local state=math.floor((state_raw or 0)+0.5)-SONG_MIXER_HEADER_MAGIC
  if state<0 or state>31 then
    v.mixer_volumes={100,100,100}; v.mixer_mute_mask=0; v.mixer_solo_lane=0
    return false
  end
  local packed=math.max(0,math.floor((volumes_raw or 0)+0.5)-1)
  local v1=packed%101; packed=math.floor(packed/101)
  local v2=packed%101; local v3=math.floor(packed/101)%101
  v.mixer_volumes={clampi(v1,0,100),clampi(v2,0,100),clampi(v3,0,100)}
  v.mixer_mute_mask=clampi(state%8,0,7)
  v.mixer_solo_lane=clampi(math.floor(state/8),0,3)
  ensure_song_mixer(v)
  return true
end

function ensure_song_index(n)
  n=clampi(n,0,MAX_SONGS-1)
  if not songs[n] then songs[n]=new_empty_song() end
  ensure_song_mixer(songs[n])
  return songs[n]
end
function set_active_song(n)
  current_song=clampi(n,0,MAX_SONGS-1)
  song=ensure_song_index(current_song)
  load_song_mixer_monitor_state(song)
end
function song_has_content(v)
  if not v then return false end
  -- Global Song metadata is persistent content. A MIDI Note assignment alone
  -- must create a valid Song slot so Song files, project state and Composition
  -- snapshots all retain the keyboard mapping.
  if clampi(v.midi_note or -1,-1,127)>=0 then return true end
  if clampi(v.song_end or -1,-1,99)~=-1 then return true end
  if clampi(v.length or 16,1,SONG_MAX_STEPS)~=16 then return true end
  ensure_song_mixer(v)
  for i=1,3 do if clampi(v.mixer_volumes[i] or 100,0,100)~=100 then return true end end
  if clampi(v.mixer_mute_mask or 0,0,7)~=0 then return true end
  if clampi(v.mixer_solo_lane or 0,0,3)~=0 then return true end
  ensure_song_lane_meta(v)
  for lane=1,3 do
    if clampi(v.lane_lengths[lane] or v.length or 16,1,SONG_MAX_STEPS)~=16 then return true end
    if clampi(v.lane_repeat[lane] or 1,1,1000)~=1 then return true end
    for ri=1,clampi(v.lane_repeat[lane] or 1,1,1000) do
      if math.abs((v.lane_speeds[lane][ri] or 1)-1)>1e-12 then return true end
    end
    for _,t in pairs((v.lanes and v.lanes[lane]) or {}) do
      if t and #song_visible_params(t)>0 then return true end
    end
  end
  return false
end

local SONG_KEYS={[1]="pattern",[2]="speed",[3]="transpose",[4]="repeat_count",[5]="pattern_start",[6]="play_length",[9]="legato",[10]="filter_owner",[11]="filter_mask",[12]="lfo1_owner",[13]="lfo2_owner",[14]="lfo3_owner",[15]="sidechain_source",[16]="sidechain_amount",[17]="delay_speed",[18]="delay_feedback",[19]="delay_volume",[20]="delay_pingpong_depth",[22]="start_offset"}
local SONG_DEFAULTS={[1]=-1,[2]=1,[3]=0,[4]=1,[5]=0,[6]=1,[9]=0,[10]=1,[11]=7,[12]=1,[13]=1,[14]=1,[15]=0,[16]=0,[17]=0,[18]=0,[19]=100,[20]=0,[22]=0}
function ensure_song_step(lane,st)
  lane=clampi(lane,1,3); st=clampi(st,0,SONG_MAX_STEPS-1)
  local t=song.lanes[lane][st]
  if not t then
    t={pattern=-1,speed=1,transpose=0,repeat_count=1,pattern_start=0,play_length=0,legato=0,filter_owner=1,filter_mask=7,lfo1_owner=1,lfo2_owner=1,lfo3_owner=1,sidechain_source=0,sidechain_amount=0,delay_speed=0,delay_feedback=0,delay_volume=100,delay_pingpong_depth=0,start_offset=0,_present={},_order={}}
    song.lanes[lane][st]=t
  end
  t._present=t._present or {}
  return t
end
function song_param_present(t,idx)
  if not t then return false end
  t._present=t._present or {}
  return t._present[idx] and true or false
end
function song_parameter_order(t)
  local present={}
  if song_param_present(t,1) then present[1]=true end
  for i=2,22 do
    if i~=5 and i~=6 and i~=7 and i~=8 and not SONG_PARAM_LIST[i].info and song_param_present(t,i) then present[i]=true end
  end
  local order={}
  local used={}
  for _,id in ipairs(t._order or {}) do
    id=tonumber(id)
    if id and present[id] and not used[id] then order[#order+1]=id; used[id]=true end
  end
  for i=1,22 do if present[i] and not used[i] then order[#order+1]=i; used[i]=true end end
  t._order=order
  return order
end
function song_visible_params(t)
  local out={}
  if not t then return out end
  -- Pattern is one indivisible block for navigation and Ctrl+Left/Right.
  -- It expands only for display into its editable controls and read-only counters.
  for _,id in ipairs(song_parameter_order(t)) do
    if id==1 then
      out[#out+1]=1
      out[#out+1]=5
      out[#out+1]=6
      out[#out+1]=21
      out[#out+1]=7
      out[#out+1]=8
    else
      out[#out+1]=id
    end
  end
  return out
end
function song_parameter_group_id(idx)
  if idx==1 or idx==5 or idx==6 or idx==7 or idx==8 or idx==21 then return 1 end
  return idx
end
function reorder_current_song_parameter(dir)
  if song_lane==4 or song_step<0 or song_step>=song_lane_length(song_lane) or is_song_speed_row() or song_is_lane_meta_row() then return end
  local t=song.lanes[song_lane][song_step]
  local idx=song_current_param_index()
  if not t or not idx then return end
  local group=song_parameter_group_id(idx)
  local order=song_parameter_order(t)
  local pos=nil
  for i,id in ipairs(order) do if id==group then pos=i; break end end
  if not pos then return end
  local target=pos+dir
  if target<1 or target>#order then return end
  order[pos],order[target]=order[target],order[pos]
  t._order=order
  local vis=song_visible_params(t)
  for i,id in ipairs(vis) do
    if id==idx then song_cell=i; break end
  end
  refresh_song_after_edit()
  speak_song_position(true)
end
function song_current_param_index()
  if song_step<0 or song_step>=song_lane_length(song_lane) then return song_cell end
  local t=song.lanes[song_lane][song_step]
  local vis=song_visible_params(t)
  if #vis==0 then return nil end
  song_cell=clampi(song_cell,1,#vis)
  local idx=vis[song_cell]
  song_logical_param=idx
  song_logical_cell=song_cell
  return idx
end

function song_cursor_preference()
  local preferred_cell=math.max(1,song_cell or song_logical_cell or 1)
  local preferred_param=song_logical_param
  if song_lane>=1 and song_lane<=3 and song_step>=0 and song_step<song_lane_length(song_lane) then
    local current=song_current_param_index()
    if current then preferred_param=current; preferred_cell=song_cell end
  end
  return preferred_param,preferred_cell
end

function restore_song_cursor_preference(preferred_param,preferred_cell)
  if song_lane<1 or song_lane>3 or song_step<0 or song_step>=song_lane_length(song_lane) then
    song_cell=math.max(1,preferred_cell or song_cell or 1)
    return
  end
  local t=song.lanes[song_lane][song_step]
  local vis=song_visible_params(t)
  if #vis==0 then song_cell=1; return end
  if preferred_param then
    for i,id in ipairs(vis) do
      if id==preferred_param then
        song_cell=i; song_logical_param=id; song_logical_cell=i; return
      end
    end
  end
  -- Keep the same horizontal list position when the exact parameter is absent,
  -- instead of always jumping to the Pattern block.
  song_cell=clampi(preferred_cell or song_cell or 1,1,#vis)
  song_logical_cell=song_cell
end
function song_end_text(v)
  v=clampi(v or -1,-1,99)
  return v<0 and "Off" or ("Song "..tostring(v))
end
function song_global_cell_count() return 1 end
function ensure_song_lane_meta(v)
  v=v or song
  if v.midi_note==nil then v.midi_note=-1 end
  if not v.lane_repeat then
    v.lane_repeat={1,1,1}
    local old=v.lane_mode or {v.mode or 0,v.mode or 0,v.mode or 0}
    for lane=1,3 do
      local m=clampi(old[lane] or 0,0,16)
      v.lane_repeat[lane]=m>=2 and m or 1
    end
  end
  if v.song_end==nil then
    local old=v.mode or 0
    v.song_end=(old==1) and -1 or -1
  end
  v.chain_loop=0
  v.song_loop_target=-1
  v.lane_lengths=v.lane_lengths or {v.length or 16,v.length or 16,v.length or 16}
  for lane=1,3 do v.lane_lengths[lane]=clampi(v.lane_lengths[lane] or v.length or 16,1,SONG_MAX_STEPS) end
  v.length=math.max(v.lane_lengths[1],v.lane_lengths[2],v.lane_lengths[3])
  v.lane_speeds=v.lane_speeds or {{1},{1},{1}}
  for lane=1,3 do
    v.lane_repeat[lane]=clampi(v.lane_repeat[lane] or 1,1,1000)
    v.lane_speeds[lane]=v.lane_speeds[lane] or {1}
    local last=1
    for i=1,1000 do
      if v.lane_speeds[lane][i]==nil then v.lane_speeds[lane][i]=last else last=v.lane_speeds[lane][i] end
    end
  end
  return v
end
function song_lane_length(lane,v)
  v=ensure_song_lane_meta(v or song)
  v.lane_lengths=v.lane_lengths or {v.length or 16,v.length or 16,v.length or 16}
  return clampi(v.lane_lengths[clampi(lane,1,3)] or v.length or 16,1,SONG_MAX_STEPS)
end
function song_recompute_length(v)
  v=ensure_song_lane_meta(v or song)
  v.lane_lengths=v.lane_lengths or {v.length or 16,v.length or 16,v.length or 16}
  v.length=math.max(song_lane_length(1,v),song_lane_length(2,v),song_lane_length(3,v))
  return v.length
end
function song_pattern_window(t)
  local p=clampi((t and t.pattern) or -1,-1,999)
  local plen=(p>=0 and ensure_pattern(p).length) or 1
  plen=clampi(plen or 1,1,MAX_STEPS)
  local start=clampi((t and t.pattern_start) or 0,0,plen-1)
  local raw_play=(t and tonumber(t.play_length)) or 0
  local play=raw_play>0 and clampi(raw_play,1,math.max(1,plen-start)) or math.max(1,plen-start)
  local next_step=start+play
  local remaining=math.max(0,plen-next_step)
  return plen,start,play,next_step,remaining
end
local song_info_step_by_lane={0,0,0}
function song_is_lane_meta_row() return song_lane<=3 and song_step==-1 end
function song_lane_meta_cell_count() return 2 end
function song_lane_info_target_step(lane)
  lane=clampi(lane or song_lane,1,3)
  local last=clampi(song_info_step_by_lane[lane] or 0,0,math.max(0,song_lane_length(lane)-1))
  local t=song.lanes[lane][last]
  if t and (t.pattern or -1)>=0 then return last,t end
  for st=0,song_lane_length(lane)-1 do
    t=song.lanes[lane][st]
    if t and (t.pattern or -1)>=0 then song_info_step_by_lane[lane]=st; return st,t end
  end
  return last,nil
end
function song_lane_meta_text(cell)
  cell=clampi(cell or song_cell,1,song_lane_meta_cell_count())
  if cell==1 then return "Op"..song_lane.." Length, "..song_lane_length(song_lane) end
  if cell==2 then return "MIDI Note, "..(((song.midi_note or -1)<0) and "Off" or note_name(song.midi_note)) end
  local st,t=song_lane_info_target_step(song_lane)
  if not t then
    local names={"","Pattern Position","Next Pattern Step","Pattern Remaining"}
    return names[cell]..", unavailable"
  end
  local plen,start,play,nx,remaining=song_pattern_window(t)
  local last=start+play-1
  if cell==2 then return "Pattern Position, step "..start.." to "..last..", from Song step "..st end
  if cell==3 then return "Next Pattern Step, "..(remaining==0 and "End" or ("step "..nx))..", from Song step "..st end
  return "Pattern Remaining, "..remaining..(remaining==1 and " step" or " steps")..", from Song step "..st
end
function song_lane_repeat_count(lane,v)
  v=ensure_song_lane_meta(v or song)
  return clampi(v.lane_repeat[lane] or 1,1,1000)
end
function song_speed_row_count() return song_lane<=3 and math.ceil(song_lane_repeat_count(song_lane)/10) or 0 end
function is_song_speed_row() return song_lane<=3 and song_step>song_lane_length(song_lane) and song_step<=song_lane_length(song_lane)+song_speed_row_count() end
function song_speed_row_number() return is_song_speed_row() and (song_step-song_lane_length(song_lane)) or 0 end
function song_speed_cell_count()
  if not is_song_speed_row() then return 0 end
  local first=(song_speed_row_number()-1)*10+1
  return math.max(0,math.min(10,song_lane_repeat_count(song_lane)-first+1))
end
function song_speed_index()
  if not is_song_speed_row() then return nil end
  return (song_speed_row_number()-1)*10+clampi(song_cell,1,math.max(1,song_speed_cell_count()))
end
function song_lane_speed_text(lane,idx)
  ensure_song_lane_meta(song)
  local v=clamp(song.lane_speeds[lane][idx] or 1,0.25,16)
  return (string.format("%.2f",v):gsub("0+$",""):gsub("%.$","")).."x"
end
function song_mask_text(v)
  v=clampi(v,0,7)
  local names={[0]="Off",[1]="Operator 1",[2]="Operator 2",[3]="Operator 1 and 2",[4]="Operator 3",[5]="Operator 1 and 3",[6]="Operator 2 and 3",[7]="All Operators"}
  return names[v]
end
function song_value_text(idx,t)
  t=t or {}
  if idx==1 then return (t.pattern or -1)<0 and "Off" or ("Pattern "..clampi(t.pattern,0,999)) end
  if idx==2 then local v=clamp(t.speed or 1,0.25,16); local z=string.format("%.2f",v):gsub("0+$",""):gsub("%.$",""); return z.."x" end
  if idx==3 then return tostring(clampi(t.transpose or 0,-96,96)) end
  if idx==4 then return tostring(clampi(t.repeat_count or 1,1,16)) end
  if idx==5 then local _,st=song_pattern_window(t); return "step "..st end
  if idx==6 then local _,_,pl=song_pattern_window(t); return tostring(pl)..(pl==1 and " step" or " steps") end
  if idx==7 then local _,_,_,nx=song_pattern_window(t); return "step "..nx end
  if idx==8 then local _,_,_,_,rm=song_pattern_window(t); return tostring(rm)..(rm==1 and " step" or " steps") end
  if idx==21 then local _,st,pl=song_pattern_window(t); return "step "..st.." to "..(st+pl-1) end
  if idx==22 then return tostring(clampi(t.start_offset or 0,0,31)).." of 32" end
  if idx==9 then return (tonumber(t.legato) or 0)>=0.5 and "On" or "Off" end
  if idx==10 then return "Operator "..clampi(t.filter_owner or 1,1,3) end
  if idx==11 then return song_mask_text(t.filter_mask or 7) end
  if idx==12 then return "Operator "..clampi(t.lfo1_owner or 1,1,3) end
  if idx==13 then return "Operator "..clampi(t.lfo2_owner or 1,1,3) end
  if idx==14 then return "Operator "..clampi(t.lfo3_owner or 1,1,3) end
  if idx==15 then local v=clampi(t.sidechain_source or 0,0,3); return v==0 and "Off" or ("Operator "..v) end
  if idx==16 then return tostring(clampi(t.sidechain_amount or 0,0,100)).." percent" end
  if idx==17 then local v=clamp(t.delay_speed or 0,0,8); if v<=0 then return "Off" end; return (string.format("%.3f",v):gsub("0+$",""):gsub("%.$", "")).." beats" end
  if idx==18 then return tostring(clampi(t.delay_feedback or 0,0,100)).." percent" end
  if idx==19 then return tostring(clampi(t.delay_volume or 100,0,100)).." percent" end
  return tostring(clampi(t.delay_pingpong_depth or 0,0,100)).." percent"
end
function speak_song_value_only()
  ensure_song_lane_meta(song)
  if song_lane==4 then
    song_cell=clampi(song_cell,1,song_global_cell_count())
    speak("Song End, "..song_end_text(song.song_end or -1))
    return
  end
  if song_is_lane_meta_row() then speak(song_lane_meta_text(song_cell)); return end
  if song_step==song_lane_length(song_lane) then
    speak("Repeat, "..song_lane_repeat_count(song_lane)); return
  end
  if is_song_speed_row() then
    local idx=song_speed_index(); speak("OP"..song_lane.." Speed "..idx..", "..song_lane_speed_text(song_lane,idx)); return
  end
  local t=song.lanes[song_lane][song_step]
  local idx=song_current_param_index()
  if not t or not idx then return end
  speak(SONG_PARAM_LIST[idx].name..", "..song_value_text(idx,t))
end

function speak_song_position(compact)
  ensure_song_lane_meta(song)
  local prefix=compact and "" or (song_lane==4 and ("Song"..current_song..". Global, ") or ("Song"..current_song..". Operator "..song_lane..", "))
  if song_lane==4 then
    song_cell=clampi(song_cell,1,song_global_cell_count())
    speak(prefix.."Song End, "..song_end_text(song.song_end or -1))
  elseif song_is_lane_meta_row() then
    speak(prefix..song_lane_meta_text(song_cell))
  elseif song_step==song_lane_length(song_lane) then
    speak(prefix.."Repeat, "..song_lane_repeat_count(song_lane))
  elseif is_song_speed_row() then
    local idx=song_speed_index()
    speak(prefix.."OP"..song_lane.." Speed "..idx..", "..song_lane_speed_text(song_lane,idx))
  else
    if song_step>=0 and song_step<song_lane_length(song_lane) then song_info_step_by_lane[song_lane]=song_step end
    local t=song.lanes[song_lane][song_step]
    local idx=song_current_param_index()
    if not idx then speak(prefix.."step "..song_step..". empty"); return end
    speak(prefix.."step "..song_step..". "..SONG_PARAM_LIST[idx].name..", "..song_value_text(idx,t))
  end
end

function write_one_song_memory(v,base)
  if not v then
    reaper.gmem_write(base,0)
    return
  end
  reaper.gmem_write(base+0,SONG_MAGIC)
  song_recompute_length(v)
  reaper.gmem_write(base+1,clampi(v.length,1,SONG_MAX_STEPS))
  for lane=1,3 do reaper.gmem_write(base+3013+lane,song_lane_length(lane,v)) end
  reaper.gmem_write(base+3017,clampi(v.midi_note or -1,-1,127))
  ensure_song_lane_meta(v)
  reaper.gmem_write(base+2,clampi(v.song_end or -1,-1,99))
  reaper.gmem_write(base+3011,0)
  reaper.gmem_write(base+3012,-1)
  reaper.gmem_write(base+3013,clampi((v._song_index~=nil and v._song_index or current_song),0,99))
  ensure_song_mixer(v)
  if v==song then save_song_mixer_monitor_state(v) end
  local mixer_volumes_packed,mixer_state_packed=pack_song_mixer_header(v)
  reaper.gmem_write(base+3018,mixer_volumes_packed)
  reaper.gmem_write(base+3019,mixer_state_packed)
  reaper.gmem_write(base+3,clampi(v.filter_owner,1,3))
  reaper.gmem_write(base+4,clampi(v.filter_mask,0,7))
  for i=1,3 do reaper.gmem_write(base+4+i,clampi((v.lfo_owner or {})[i] or 1,1,3)) end
  for lane=1,3 do
    reaper.gmem_write(base+7+lane,clampi(v.lane_repeat[lane] or 1,1,1000))
    for ri=1,1000 do reaper.gmem_write(base+10+(lane-1)*1000+ri,clamp(v.lane_speeds[lane][ri] or 1,0.25,16)) end
  end
  for lane=1,3 do
    local lb=base+SONG_HEADER_SIZE+(lane-1)*SONG_LANE_STRIDE
    for st=0,SONG_MAX_STEPS-1 do
      local t=v.lanes[lane][st]
      local b=lb+st*SONG_STEP_STRIDE
      local fo=t and song_param_present(t,10) and clampi(t.filter_owner or 1,1,3) or 0
      local fm=t and song_param_present(t,11) and (clampi(t.filter_mask or 7,0,7)+1) or 0
      local l1=t and song_param_present(t,12) and clampi(t.lfo1_owner or 1,1,3) or 0
      local l2=t and song_param_present(t,13) and clampi(t.lfo2_owner or 1,1,3) or 0
      local l3=t and song_param_present(t,14) and clampi(t.lfo3_owner or 1,1,3) or 0
      reaper.gmem_write(b,(t and ((t.pattern or -1)+1) or 0)+fo*2048)
      reaper.gmem_write(b+1,(t and (t.speed or 1) or 1)+fm*32)
      reaper.gmem_write(b+2,clampi((t and (t.transpose or 0) or 0),-96,96)+128+l1*256)
      reaper.gmem_write(b+3,(t and (t.repeat_count or 1) or 1)+l2*32)
      reaper.gmem_write(b+4,(t and (t.legato or 0) or 0)+l3*2)
      local scs=t and song_param_present(t,15) and (clampi(t.sidechain_source or 0,0,3)+1) or 0
      local sca=t and song_param_present(t,16) and (clampi(t.sidechain_amount or 0,0,100)+1) or 0
      reaper.gmem_write(b+5,scs*2+sca*10)
      local ds=t and song_param_present(t,17) and (math.floor(clamp(t.delay_speed or 0,0,8)*8+0.5)+1) or 0
      local df=t and song_param_present(t,18) and (clampi(t.delay_feedback or 0,0,100)+1) or 0
      local dv=t and song_param_present(t,19) and (clampi(t.delay_volume or 100,0,100)+1) or 0
      local dp=t and song_param_present(t,20) and (clampi(t.delay_pingpong_depth or 0,0,100)+1) or 0
      reaper.gmem_write(b+6,ds)
      reaper.gmem_write(b+7,df)
      reaper.gmem_write(b+8,dv)
      reaper.gmem_write(b+9,dp)
      local _,pst,pl=song_pattern_window(t)
      local start_offset_code=t and song_param_present(t,22) and (clampi(t.start_offset or 0,0,31)+1) or 0
      reaper.gmem_write(b+10,t and (pst*SONG_START_PACK_BASE+start_offset_code) or 0)
      reaper.gmem_write(b+11,t and pl or 0)
    end
  end
end

function rebuild_song_midi_map()
  reaper.gmem_attach(GMEM_NS)
  for n=0,127 do reaper.gmem_write(SONG_MIDI_MAP_BASE+n,0) end
  local owner={}
  for sn=0,MAX_SONGS-1 do
    local v=songs[sn]
    if v then
      ensure_song_lane_meta(v)
      local note=clampi(v.midi_note or -1,-1,127)
      if note>=0 then owner[note]=sn end
    end
  end
  for note,sn in pairs(owner) do reaper.gmem_write(SONG_MIDI_MAP_BASE+note,sn+1) end
end

function update_midi_trigger_page_mode()
  -- MIDI triggering is permanently Song-only in the JSFX Tracker.
  -- Page changes must never rewrite or gate the Song MIDI path.
end
function write_all_songs_memory()
  reaper.gmem_attach(GMEM_NS)
  for i=0,MAX_SONGS-1 do
    local v=songs[i]
    if v then v._song_index=i end
    write_one_song_memory(v,SONG_BANK_BASE+i*SONG_BANK_STRIDE)
  end
  rebuild_song_midi_map()
end

function prime_song_live_delay_state(v)
  -- Reconstruct the delay state from Song row 0 through the first playable
  -- Pattern row.  FX-only rows are valid commands and must not be skipped.
  -- Missing fields keep the already latched value; only an explicit Speed Off
  -- switches the delay off.
  reaper.gmem_attach(GMEM_NS)
  for lane=1,3 do
    local b=DELAY_LIVE_BASE+(lane-1)*DELAY_LIVE_STRIDE
    local speed=clamp(reaper.gmem_read(b+0) or 0,0,8)
    local feedback=clampi(reaper.gmem_read(b+1) or 0,0,100)
    local volume=clampi(reaper.gmem_read(b+2) or 100,0,100)
    local ping=clampi(reaper.gmem_read(b+3) or 0,0,100)
    local changed=false
    local len=song_lane_length(lane,v)
    for st=0,len-1 do
      local t=v.lanes[lane][st]
      if t then
        local present=t._present or {}
        if present[17] then speed=clamp(t.delay_speed or 0,0,8); changed=true end
        if present[18] then feedback=clampi(t.delay_feedback or 0,0,100); changed=true end
        if present[19] then volume=clampi(t.delay_volume or 100,0,100); changed=true end
        if present[20] then ping=clampi(t.delay_pingpong_depth or 0,0,100); changed=true end
        if (t.pattern or -1)>=0 then break end
      end
    end
    if changed then
      reaper.gmem_write(b+0,speed)
      reaper.gmem_write(b+1,feedback)
      reaper.gmem_write(b+2,volume)
      reaper.gmem_write(b+3,ping)
      reaper.gmem_write(b+4,math.floor(reaper.gmem_read(b+4) or 0)+1)
    end
  end
end

function prime_song_live_sidechain_state(v)
  -- A new Song launch starts with sidechain Off, then reconstructs explicit F3
  -- Source/Amount commands from row 0 through the first playable Pattern row.
  reaper.gmem_attach(GMEM_NS)
  for lane=1,3 do
    local source,amount=0,0
    local len=song_lane_length(lane,v)
    for st=0,len-1 do
      local t=v.lanes[lane][st]
      if t then
        local present=t._present or {}
        if present[15] then source=clampi(t.sidechain_source or 0,0,3) end
        if present[16] then amount=clampi(t.sidechain_amount or 0,0,100) end
        if (t.pattern or -1)>=0 then break end
      end
    end
    -- Either zero means fully Off; never carry the other component from history.
    if source<=0 or amount<=0 then source=0; amount=0 end
    local b=SIDECHAIN_LIVE_BASE+(lane-1)*SIDECHAIN_LIVE_STRIDE
    reaper.gmem_write(b+0,source)
    reaper.gmem_write(b+1,amount)
    reaper.gmem_write(b+2,math.floor(reaper.gmem_read(b+2) or 0)+1)
  end
end

function publish_song_mixer_state()
  -- Publish values only. The Tracker owns LIVE_BASE+7 and raises it only when
  -- a real Song is running, so project-start restore never depends on F3 focus.
  ensure_song_mixer(song)
  save_song_mixer_monitor_state(song)
  reaper.gmem_attach(GMEM_NS)
  for i=1,3 do reaper.gmem_write(SONG_MIXER_LIVE_BASE+i-1,clampi(song.mixer_volumes[i] or 100,0,100)) end
  reaper.gmem_write(SONG_MIXER_LIVE_BASE+3,clampi(song.mixer_mute_mask or 0,0,7))
  reaper.gmem_write(SONG_MIXER_LIVE_BASE+4,clampi(song.mixer_solo_lane or 0,0,3))
  reaper.gmem_write(SONG_MIXER_LIVE_BASE+6,SONG_MIXER_MAGIC)
  reaper.gmem_write(SONG_MIXER_LIVE_BASE+5,math.floor(reaper.gmem_read(SONG_MIXER_LIVE_BASE+5) or 0)+1)
end

function write_active_song_mixer_headers()
  ensure_song_mixer(song)
  save_song_mixer_monitor_state(song)
  reaper.gmem_attach(GMEM_NS)
  local bank=SONG_BANK_BASE+current_song*SONG_BANK_STRIDE
  local volumes_packed,state_packed=pack_song_mixer_header(song)
  reaper.gmem_write(SONG_BASE+3018,volumes_packed)
  reaper.gmem_write(SONG_BASE+3019,state_packed)
  reaper.gmem_write(bank+3018,volumes_packed)
  reaper.gmem_write(bank+3019,state_packed)
end

function save_current_song_project_slot()
  if not reaper.SetProjExtState then return end
  save_song_mixer_monitor_state(song)
  songs[current_song]=song
  reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"active_song",tostring(current_song))
  reaper.SetProjExtState(0,PROJECT_EXT_SECTION,string.format("song_%02d",current_song),serialize_song(song))
  reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"song_data",serialize_song(song))
  reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"song_slots_override_composition","1")
  -- Mixer edits must become part of the RPP state seen by the Startup Loader.
  -- Explicitly dirty the project so REAPER asks to save these changes.
  if reaper.MarkProjectDirty then reaper.MarkProjectDirty(0) end
end

function write_song_memory(run)
  if run then biscot_set_core_midi_mode_for_page(3) end
  reaper.gmem_attach(GMEM_NS)
  -- Keep the active Lua object in the 100-Song bank before playback. Song
  -- chaining is performed entirely by the Tracker JSFX, so every destination
  -- must already exist in GMEM when the current Song reaches its end.
  songs[current_song]=song
  song._song_index=current_song
  -- The 100-slot bank is persistent. Rewriting every loaded Song here makes
  -- Play progressively slower after a Composition load. Sync only the live slot.
  write_one_song_memory(song,SONG_BASE)
  write_one_song_memory(song,SONG_BANK_BASE+current_song*SONG_BANK_STRIDE)
  publish_song_mixer_state()
  if run then
    prime_song_live_delay_state(song)
    prime_song_live_sidechain_state(song)
  end
  rebuild_song_midi_map()
  reaper.gmem_write(LUA_BASE+0,LUA_MAGIC)
  reaper.gmem_write(LUA_BASE+2,1)
  reaper.gmem_write(LUA_BASE+3,run and 2 or 0)
  lua_serial=lua_serial+1
  reaper.gmem_write(LUA_BASE+1,lua_serial)
  -- Re-publish the reconstructed state for a few controller cycles after
  -- playback becomes active.  This is an additional handshake for unusual FX
  -- ordering or anticipative-processing cases; identical Speed values do not
  -- clear the delay line again.
  if run then
    song_delay_prime_pending=4
    song_mixer_prime_pending=8
  else
    song_delay_prime_pending=0
    song_mixer_prime_pending=0
  end
end
function serialize_song(song_obj)
  local song= song_obj or song
  ensure_song_lane_meta(song)
  ensure_song_mixer(song)
  if song_obj==nil or song==songs[current_song] then save_song_mixer_monitor_state(song) end
  local out={"BISCOT1_TRACKER_SONG_V1","song_name="..tostring(song.name or ""):gsub("[\r\n\t]"," "),"song_end_direct=1","length="..song.length,"lane_lengths="..table.concat(song.lane_lengths or {song.length,song.length,song.length},","),"midi_note="..clampi(song.midi_note or -1,-1,127),"song_end="..(song.song_end or 0),"chain_loop=0","song_loop_target=-1","global_fields_v=1","pattern_window_v=1","exact_pattern_windows_v=1","lane_repeat="..table.concat(song.lane_repeat,","),"mixer_volumes="..table.concat(song.mixer_volumes,","),"mixer_mute_mask="..clampi(song.mixer_mute_mask or 0,0,7),"mixer_solo_lane="..clampi(song.mixer_solo_lane or 0,0,3),"filter_owner="..song.filter_owner,"filter_mask="..song.filter_mask,
    "lfo_owner="..table.concat(song.lfo_owner,",")}
  for lane=1,3 do for ri=1,song_lane_repeat_count(lane,song) do out[#out+1]=table.concat({"lane_speed",lane,ri,song.lane_speeds[lane][ri] or 1},"\t") end end
  for lane=1,3 do for st=0,song_lane_length(lane,song)-1 do local t=song.lanes[lane][st]
    if t and #song_visible_params(t)>0 then
      local mask=0
      for i=1,22 do if song_param_present(t,i) then mask=mask+(2^(i-1)) end end
      out[#out+1]=table.concat({"lane",lane,st,t.pattern or -1,t.speed or 1,t.transpose or 0,t.repeat_count or 1,t.pattern_start or 0,t.play_length or 0,t.legato or 0,mask,t.filter_owner or 1,t.filter_mask or 7,t.lfo1_owner or 1,t.lfo2_owner or 1,t.lfo3_owner or 1,t.sidechain_source or 0,t.sidechain_amount or 0,t.delay_speed or 0,t.delay_feedback or 0,t.delay_volume or 100,t.delay_pingpong_depth or 0,t.start_offset or 0},"\t")
    end
  end end
  return table.concat(out,"\n")
end

function sanitize_song_filename(name)
  name=tostring(name or ""):gsub('[\\/:*?"<>|]',"-"):gsub("^%s+",""):gsub("%s+$","")
  name=name:gsub("%.txt$","")
  if name=="" then return "" end
  if not name:lower():match("^song%-") then name="Song-"..name end
  return name..".txt"
end
function automatic_song_filename()
  return "Song-"..os.date("%Y%m%d-%H%M%S")..".txt"
end
function unique_song_filename(name)
  name=sanitize_song_filename(name)
  if name=="" then name=automatic_song_filename() end
  local path=join_path(songs_dir(),name)
  if not file_exists(path) then return name end
  local stem=name:gsub("%.txt$","")
  local n=2
  repeat
    name=string.format("%s-%03d.txt",stem,n)
    path=join_path(songs_dir(),name)
    n=n+1
  until not file_exists(path)
  return name
end
function save_all_patterns_for_song()
  ensure_dir(patterns_dir())
  local keep_pattern=current_pattern
  local keep_active_file=read_text_file(active_pattern_file_path()) or ""
  for p=0,MAX_PATTERNS-1 do
    local pat=patterns[p]
    if pat and pattern_has_content(pat) then
      save_pattern_txt(p)
    end
  end
  write_text_file(active_pattern_path(),tostring(keep_pattern))
  write_text_file(active_pattern_file_path(),keep_active_file)
  if reaper.SetProjExtState then
    reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"active_pattern",tostring(keep_pattern))
    reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"active_pattern_file",keep_active_file)
  end
end
function clear_user_txt_files(dir,kind)
  dir=norm_path(dir)
  if dir=="" then return end
  local i=0
  local paths={}
  while true do
    local fn=reaper.EnumerateFiles(dir,i); if not fn then break end
    local path=join_path(dir,fn)
    if tostring(fn):lower():match("%.txt$") and f5_txt_kind(path)==kind then
      paths[#paths+1]=path
    end
    i=i+1
  end
  for _,path in ipairs(paths) do os.remove(path) end
end

function song_package_folder_name(filename)
  local stem=tostring(filename or ""):gsub("%.txt$",""):gsub("^%s+",""):gsub("%s+$","")
  if stem=="" then stem="Song-"..os.date("%Y%m%d-%H%M%S") end
  return stem
end

function write_song_package(base_dir,filename,song_text)
  base_dir=norm_path(base_dir)
  filename=sanitize_song_filename(filename)
  if base_dir=="" or filename=="" then return false,"" end
  -- Replace the obsolete flat Song copy with the self-contained package.
  os.remove(join_path(base_dir,filename))
  local root=join_path(base_dir,song_package_folder_name(filename))
  ensure_dir(root)
  local song_path=join_path(root,filename)
  local temp_path=song_path..".tmp"
  os.remove(temp_path)
  if not write_text_file(temp_path,song_text) then return false,song_path end
  os.remove(song_path)
  local renamed=os.rename(temp_path,song_path)
  if not renamed then
    os.remove(temp_path)
    if not write_text_file(song_path,song_text) then return false,song_path end
  end
  local verify=parse_song_text(read_text_file(song_path))
  if not verify then return false,song_path end
  -- Same proven structure as a Composition: one package folder with its
  -- human-readable Song TXT and an exact Patterns folder captured from GMEM.
  if not save_composition_patterns(root) then return false,song_path end
  return true,song_path
end

function save_song_txt_with_name()
  local suggested=automatic_song_filename()
  local ok,input=reaper.GetUserInputs("Save song",1,"Song name:",suggested:gsub("%.txt$",""))
  restore_controller_focus_after_dialog()
  if not ok then return false,"" end
  local library_ready=ensure_library_root()

  local container_filename=sanitize_song_filename(input)
  if container_filename=="" then return false,"" end
  save_song_mixer_monitor_state(song)
  songs[current_song]=song
  local previous_name=song.name
  song.name=container_filename:gsub("%.txt$","")
  local raw_song_text=serialize_song(song)
  local verify=parse_song_text(raw_song_text)
  if not verify then song.name=previous_name; return false,"" end
  local song_text=serialize_song(verify)

  -- Keep the ordinary Project/Default container current for Startup fallback,
  -- then create a self-contained Song package beside it.
  rebuild_midi_pattern_bank()
  save_all_patterns_for_song()
  local saved,container_path=write_song_package(songs_dir(),container_filename,song_text)
  if not saved then song.name=previous_name; return false,container_path end
  write_text_file(active_song_file_path(),norm_path(container_path))
  save_current_song_project_slot()

  local returned_path=container_path
  if library_ready then
    local lib_saved,library_song_path=write_song_package(library_songs_dir(),container_filename,song_text)
    if not lib_saved then
      song.name=previous_name
      return false,library_song_path
    end
    returned_path=library_song_path
  end

  set_song_name(current_song,container_filename)
  update_controller_title()
  return true,returned_path
end
function composition_song_at(index)
  index=clampi(index or 0,0,MAX_SONGS-1)
  local v=songs[index]
  if v then return v end
  if reaper.GetProjExtState then
    local _,txt=reaper.GetProjExtState(0,PROJECT_EXT_SECTION,string.format("song_%02d",index))
    v=parse_song_text(txt)
    if v then songs[index]=v; return v end
  end
  return nil
end

local BANK_LOGICAL_ID_BY_RESOLVED=nil
function composition_build_bank_id_map()
  if BANK_LOGICAL_ID_BY_RESOLVED then return end
  BANK_LOGICAL_ID_BY_RESOLVED={}
  for _,pp in ipairs(PARAMS) do
    BANK_LOGICAL_ID_BY_RESOLVED[pp.id]=pp.id
    if pp.map and pp.map[1] then BANK_LOGICAL_ID_BY_RESOLVED[1000+pp.map[1]]=pp.id end
  end
end

function composition_pattern_text_from_gmem(pi)
  pi=clampi(pi or 0,0,MAX_PATTERNS-1)
  local base=math.floor(reaper.gmem_read(BANK_DIR_BASE+pi) or 0)
  if base<=0 or math.floor(reaper.gmem_read(base+0) or 0)~=BANK_MAGIC then return nil end
  if clampi(reaper.gmem_read(base+1) or pi,0,MAX_PATTERNS-1)~=pi then return nil end
  composition_build_bank_id_map()
  local length=clampi(reaper.gmem_read(base+2) or 16,1,MAX_STEPS)
  local rc=clampi(reaper.gmem_read(base+3) or 1,1,1000)
  local step_dir_rel=math.floor(reaper.gmem_read(base+4) or 0)
  if step_dir_rel<5 or step_dir_rel>100000 then return nil end
  local midi_note=-1
  for note=0,127 do if math.floor(reaper.gmem_read(MIDI_MAP_BASE+note) or 0)==pi+1 then midi_note=note; break end end
  local out={"BISCOT1_TRACKER_RAM_V2","pattern="..pi,"length="..length,"repeat="..rc,"midi_note="..midi_note,"release_arp="..clampi(reaper.gmem_read(BANK_RELEASE_ARP_BASE+pi) or 0,0,1)}
  for ri=1,rc do
    out[#out+1]="speed\t"..ri.."\t"..tostring(clamp(reaper.gmem_read(base+4+ri) or 1,0.25,16))
    out[#out+1]="transpose\t"..ri.."\t"..tostring(clampi(reaper.gmem_read(base+4+rc+ri) or 0,-96,96))
  end
  for st=0,MAX_STEPS-1 do
    local rel=math.floor(reaper.gmem_read(base+step_dir_rel+st) or 0)
    if rel>0 then
      local ptr=base+rel
      local count=clampi(reaper.gmem_read(ptr) or 0,0,MAX_CELLS)
      local grouped={{},{},{}}
      for ci=0,count-1 do
        local rid=math.floor(reaper.gmem_read(ptr+1+ci*2) or 0)
        local val=reaper.gmem_read(ptr+2+ci*2) or 0
        local lid=BANK_LOGICAL_ID_BY_RESOLVED[rid] or rid
        local pp=PARAM_BY_ID[lid]
        local tab=pp and clampi(pp.tab or 1,1,3) or 1
        grouped[tab][#grouped[tab]+1]={id=lid,value=val}
      end
      for tab=1,3 do if #grouped[tab]>0 then
        out[#out+1]="step\t"..st.."\t"..tab
        for _,c in ipairs(grouped[tab]) do out[#out+1]="cell\t"..c.id.."\t"..tostring(c.value) end
      end end
    end
  end
  return table.concat(out,"\n")
end

function composition_song_from_gmem(sn)
  sn=clampi(sn or 0,0,MAX_SONGS-1)
  local base=SONG_BANK_BASE+sn*SONG_BANK_STRIDE
  if math.floor(reaper.gmem_read(base+0) or 0)~=SONG_MAGIC then return new_empty_song() end
  local v=new_empty_song()
  v._song_index=sn
  v.length=clampi(reaper.gmem_read(base+1) or 16,1,SONG_MAX_STEPS)
  v.lane_lengths={clampi(reaper.gmem_read(base+3014) or v.length,1,SONG_MAX_STEPS),clampi(reaper.gmem_read(base+3015) or v.length,1,SONG_MAX_STEPS),clampi(reaper.gmem_read(base+3016) or v.length,1,SONG_MAX_STEPS)}
  v.midi_note=clampi(reaper.gmem_read(base+3017) or -1,-1,127)
  song_recompute_length(v)
  v.song_end=clampi(reaper.gmem_read(base+2) or -1,-1,99)
  v.chain_loop=clampi(reaper.gmem_read(base+3011) or 0,0,1)
  v.song_loop_target=clampi(reaper.gmem_read(base+3012) or -1,-1,99)
  v.filter_owner=clampi(reaper.gmem_read(base+3) or 1,1,3)
  v.filter_mask=clampi(reaper.gmem_read(base+4) or 7,0,7)
  v.lfo_owner={clampi(reaper.gmem_read(base+5) or 1,1,3),clampi(reaper.gmem_read(base+6) or 1,1,3),clampi(reaper.gmem_read(base+7) or 1,1,3)}
  unpack_song_mixer_header(v,reaper.gmem_read(base+3018),reaper.gmem_read(base+3019))
  for lane=1,3 do
    local rc=clampi(reaper.gmem_read(base+7+lane) or 1,1,1000)
    v.lane_repeat[lane]=rc
    v.lane_speeds[lane]={}
    for ri=1,rc do v.lane_speeds[lane][ri]=clamp(reaper.gmem_read(base+10+(lane-1)*1000+ri) or 1,0.25,16) end
    local lb=base+SONG_HEADER_SIZE+(lane-1)*SONG_LANE_STRIDE
    for st=0,v.length-1 do
      local b=lb+st*SONG_STEP_STRIDE
      local x0=reaper.gmem_read(b) or 0
      local x1=reaper.gmem_read(b+1) or 0
      local x2=reaper.gmem_read(b+2) or 0
      local x3=reaper.gmem_read(b+3) or 0
      local x4=reaper.gmem_read(b+4) or 0
      local x5=reaper.gmem_read(b+5) or 0
      local x6=reaper.gmem_read(b+6) or 0
      local x7=reaper.gmem_read(b+7) or 0
      local x8=reaper.gmem_read(b+8) or 0
      local x9=reaper.gmem_read(b+9) or 0
      local x10=reaper.gmem_read(b+10) or 0
      local x11=reaper.gmem_read(b+11) or 0
      local packed_start=clampi(x10,0,MAX_STEPS*SONG_START_PACK_BASE-1)
      local pattern_start_value=math.floor(packed_start/SONG_START_PACK_BASE)
      local start_offset_code=packed_start%SONG_START_PACK_BASE
      local pattern_raw=math.floor(x0)%2048
      local fo=math.floor(x0/2048)%4
      local fm=math.floor(x1/32)%32
      local l1=math.floor(x2/256)%8
      local l2=math.floor(x3/32)%8
      local l3=math.floor(x4/2)%8
      local scs=math.floor(x5/2)%5
      local sca=math.floor(x5/10)%102
      local speed_value=x1-math.floor(x1/32)*32
      local transpose_value=(x2-math.floor(x2/256)*256)-128
      local repeat_value=math.floor(x3)%32
      local legato_value=math.floor(x4)%2
      local present={}
      -- Restore the exact SONG_PARAM_LIST indices. The previous decoder wrote
      -- F4 presence into indices 7..17, so Filter/LFO/FX parameters vanished
      -- or appeared under unrelated F3 cells after loading a Composition.
      present[1]=pattern_raw>0
      present[2]=math.abs(speed_value-1)>0.0000001
      present[3]=math.abs(transpose_value)>0.0000001
      present[4]=repeat_value~=1
      present[9]=legato_value~=0
      present[10]=fo>0
      present[11]=fm>0
      present[12]=l1>0
      present[13]=l2>0
      present[14]=l3>0
      present[15]=scs>0
      present[16]=sca>0
      present[17]=x6>0
      present[18]=x7>0
      present[19]=x8>0
      present[20]=x9>0
      present[22]=start_offset_code>0
      local any=false; for i=1,22 do if present[i] then any=true; break end end
      if any then
        v.lanes[lane][st]={
          pattern=pattern_raw-1,
          speed=clamp(speed_value,0.25,16),
          transpose=clampi(transpose_value,-96,96),
          repeat_count=clampi(repeat_value,1,16),
          pattern_start=clampi(pattern_start_value,0,MAX_STEPS-1),
          play_length=(function()
            local pl=clampi(x11,1,MAX_STEPS)
            local _,exact_state=reaper.GetProjExtState(0,PROJECT_EXT_SECTION,"pattern_window_exact_v")
            local pidx=pattern_raw-1
            local plen=(pidx>=0 and ensure_pattern(pidx).length) or 1
            plen=clampi(plen or 1,1,MAX_STEPS)
            if tostring(exact_state or "")~="1" and pattern_start_value==0 and plen>=3 and pl==plen-2 then pl=plen end
            return pl
          end)(),
          start_offset=start_offset_code>0 and clampi(start_offset_code-1,0,31) or 0,
          legato=clampi(legato_value,0,1),
          filter_owner=clampi(fo,1,3),filter_mask=clampi(fm-1,0,7),
          lfo1_owner=clampi(l1,1,3),lfo2_owner=clampi(l2,1,3),lfo3_owner=clampi(l3,1,3),
          sidechain_source=clampi(scs-1,0,3),sidechain_amount=clampi(sca-1,0,100),
          delay_speed=x6>0 and clamp((x6-1)/8,0,8) or 0,
          delay_feedback=x7>0 and clampi(x7-1,0,100) or 0,
          delay_volume=x8>0 and clampi(x8-1,0,100) or 100,
          delay_pingpong_depth=x9>0 and clampi(x9-1,0,100) or 0,
          _present=present}
      end
    end
  end
  ensure_song_lane_meta(v)
  return v
end

function composition_number_text(v)
  v=tonumber(v) or 0
  if math.abs(v)<0.000000000000001 then v=0 end
  return string.format("%.17g",v)
end

function composition_capture_runs(start_addr,count,prefix,extra,out)
  count=math.max(0,math.floor(count or 0))
  if count<=0 then return end
  local run_start=0
  local run_value=reaper.gmem_read(start_addr) or 0
  local run_count=1
  for i=1,count-1 do
    local v=reaper.gmem_read(start_addr+i) or 0
    if v==run_value then
      run_count=run_count+1
    else
      out[#out+1]=table.concat({prefix,extra or "",run_start,run_count,composition_number_text(run_value)},"\t")
      run_start=i; run_value=v; run_count=1
    end
  end
  out[#out+1]=table.concat({prefix,extra or "",run_start,run_count,composition_number_text(run_value)},"\t")
end

function capture_composition_ram_snapshot(name)
  -- Commit only the live controller RAM to the Tracker banks. No disk,
  -- Library, filenames or project-cache lookup is used by the snapshot.
  reaper.gmem_attach(GMEM_NS)
  -- The indexed Pattern bank is already kept current by every edit.
  -- Do not rebuild it here: rebuilding from the Lua cache can discard slots
  -- that exist only in the Tracker JSFX RAM.
  save_song_mixer_monitor_state(song)
  songs[current_song]=song
  song._song_index=current_song
  write_one_song_memory(song,SONG_BANK_BASE+current_song*SONG_BANK_STRIDE)
  rebuild_song_midi_map()
  write_one_song_memory(song,SONG_BASE)

  local out={
    "BISCOT1_TRACKER_COMPOSITION_V8_RAW_GMEM",
    "name="..tostring(name or "Composition"),
    "active_pattern="..clampi(current_pattern or 0,0,MAX_PATTERNS-1),
    "active_song="..clampi(current_song or 0,0,MAX_SONGS-1),
    "pattern_data_base="..BANK_DATA_BASE,
    "song_bank_base="..SONG_BANK_BASE,
    "song_bank_stride="..SONG_BANK_STRIDE
  }

  composition_capture_runs(BANK_CTRL_BASE,4,"pattern_ctrl","",out)
  composition_capture_runs(BANK_DIR_BASE,MAX_PATTERNS,"pattern_dir","",out)
  composition_capture_runs(BANK_RELEASE_ARP_BASE,MAX_PATTERNS,"pattern_release","",out)
  composition_capture_runs(MIDI_MAP_BASE,128,"midi_map","",out)
  local cursor=math.floor(reaper.gmem_read(BANK_CTRL_BASE+3) or BANK_DATA_BASE)
  cursor=clampi(cursor,BANK_DATA_BASE,BANK_MAX_ADDRESS)
  local data_size=math.max(0,cursor-BANK_DATA_BASE)
  out[#out+1]="pattern_data_size="..data_size
  composition_capture_runs(BANK_DATA_BASE,data_size,"pattern_data","",out)

  for sn=0,MAX_SONGS-1 do
    local base=SONG_BANK_BASE+sn*SONG_BANK_STRIDE
    if math.floor(reaper.gmem_read(base) or 0)==SONG_MAGIC then
      local len=clampi(reaper.gmem_read(base+1) or 1,1,SONG_MAX_STEPS)
      out[#out+1]=table.concat({"song_slot",sn,len},"\t")
      composition_capture_runs(base,SONG_HEADER_SIZE,"song_header",sn,out)
      for lane=1,3 do
        local lb=base+SONG_HEADER_SIZE+(lane-1)*SONG_LANE_STRIDE
        composition_capture_runs(lb,len*SONG_STEP_STRIDE,"song_lane",sn..","..lane,out)
      end
    end
  end
  return {name=name,active_song=current_song,active_pattern=current_pattern,raw_text=table.concat(out,"\n").."\n"}
end

function clear_all_txt_files(dir)
  dir=norm_path(dir)
  if dir=="" then return end
  local i=0
  local paths={}
  while true do
    local fn=reaper.EnumerateFiles(dir,i)
    if not fn then break end
    if tostring(fn):lower():match("%.txt$") then paths[#paths+1]=join_path(dir,fn) end
    i=i+1
  end
  for _,path in ipairs(paths) do os.remove(path) end
end

function composition_unique_name(base_name,local_base,library_base)
  local candidate=base_name
  local n=1
  while true do
    local exists_local=file_exists(join_path(join_path(local_base,candidate),"composition.txt"))
    local exists_library=false
    if library_base and library_base~="" then
      exists_library=file_exists(join_path(join_path(library_base,candidate),"composition.txt"))
    end
    if not exists_local and not exists_library then return candidate end
    n=n+1
    candidate=base_name.." "..n
  end
end


function composition_pattern_filename(pi)
  pi=clampi(pi or 0,0,MAX_PATTERNS-1)
  local raw=tostring(pattern_names[pi] or "")
  raw=raw:gsub("%.txt$",""):gsub('[\\/:*?"<>|]',"-"):gsub("^%s+",""):gsub("%s+$","")
  raw=raw:gsub("^[Pp]attern[%s%-%_]*%d+[%s%-%_]*","")
  if raw=="" then return string.format("Pattern-%03d.txt",pi) end
  return string.format("Pattern-%03d-%s.txt",pi,raw)
end

function save_composition_patterns(root)
  local dir=join_path(root,"Patterns")
  ensure_dir(dir)
  clear_all_txt_files(dir)
  local saved=0
  for pi=0,MAX_PATTERNS-1 do
    local txt=composition_pattern_text_from_gmem(pi)
    if txt and txt:find("BISCOT1_TRACKER_RAM_V2",1,true) then
      if write_text_file(join_path(dir,composition_pattern_filename(pi)),txt) then saved=saved+1 end
    end
  end
  write_text_file(join_path(dir,"active_pattern.txt"),tostring(clampi(current_pattern or 0,0,MAX_PATTERNS-1)))
  return saved
end

function write_composition_copy(root,name,snap)
  ensure_dir(root)
  snap=snap or capture_composition_ram_snapshot(name)
  local composition_path=join_path(root,"composition.txt")
  local temp_path=composition_path..".tmp"
  os.remove(temp_path)
  if not write_text_file(temp_path,snap.raw_text or "") then return false end
  os.remove(composition_path)
  local renamed=os.rename(temp_path,composition_path)
  if not renamed then
    os.remove(temp_path)
    if not write_text_file(composition_path,snap.raw_text or "") then return false end
  end
  local verify=read_text_file(composition_path)
  if not (verify and verify:find("BISCOT1_TRACKER_COMPOSITION_V8_RAW_GMEM",1,true)~=nil) then return false end
  -- Keep human-readable, exact Pattern TXT copies beside the raw Composition.
  -- These are captured from the live GMEM bank, so unsaved F1 edits are included.
  save_composition_patterns(root)
  return true
end

function save_composition_with_name()
  local suggested="Composition-"..os.date("%Y%m%d-%H%M%S")
  local ok,input=reaper.GetUserInputs("Save composition",1,"Composition name:",suggested)
  restore_controller_focus_after_dialog()
  if not ok then return false,"" end
  local name=tostring(input or ""):gsub('[\\/:*?"<>|]',"-"):gsub("^%s+",""):gsub("%s+$","")
  if name=="" then return false,"" end

  ensure_library_root()
  local local_base=compositions_dir()
  local lib_base=library_compositions_dir()
  name=composition_unique_name(name,local_base,lib_base)
  local snap=capture_composition_ram_snapshot(name)

  local local_root=join_path(local_base,name)
  if not write_composition_copy(local_root,name,snap) then return false,local_root end
  if reaper.SetProjExtState then
    reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"composition_raw",snap.raw_text or "")
    -- The raw Composition now contains the current Song mixer states.
    reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"song_slots_override_composition","0")
    reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"pattern_slots_override_composition","0")
  end

  if lib_base~="" then
    local library_root=join_path(lib_base,name)
    if not write_composition_copy(library_root,name,snap) then return false,library_root end
    return true,library_root
  end
  return true,local_root
end

function composition_apply_run(start_addr,rel,count,value)
  rel=math.max(0,math.floor(tonumber(rel) or 0))
  count=math.max(0,math.floor(tonumber(count) or 0))
  value=tonumber(value) or 0
  for i=0,count-1 do reaper.gmem_write(start_addr+rel+i,value) end
end

function load_composition_txt(path)
  local txt=read_text_file(path)
  if not txt then return false end
  if not txt:find("BISCOT1_TRACKER_COMPOSITION_V8_RAW_GMEM",1,true) then
    return false
  end
  reaper.gmem_attach(GMEM_NS)

  local old_cursor=math.floor(reaper.gmem_read(BANK_CTRL_BASE+3) or BANK_DATA_BASE)
  local new_size=math.max(0,tonumber(txt:match("pattern_data_size=(%d+)")) or 0)
  local clear_size=math.max(0,math.max(old_cursor-BANK_DATA_BASE,new_size))
  clear_size=math.min(clear_size,BANK_MAX_ADDRESS-BANK_DATA_BASE)
  for i=0,3 do reaper.gmem_write(BANK_CTRL_BASE+i,0) end
  for i=0,MAX_PATTERNS-1 do
    reaper.gmem_write(BANK_DIR_BASE+i,0)
    reaper.gmem_write(BANK_RELEASE_ARP_BASE+i,0)
  end
  for i=0,127 do reaper.gmem_write(MIDI_MAP_BASE+i,0) end
  for i=0,clear_size-1 do reaper.gmem_write(BANK_DATA_BASE+i,0) end

  local song_lengths={}
  for line in (txt.."\n"):gmatch("([^\n]*)\n") do
    local sn,len=line:match("^song_slot\t(%d+)\t(%d+)$")
    if sn then song_lengths[clampi(tonumber(sn),0,MAX_SONGS-1)]=clampi(tonumber(len),1,SONG_MAX_STEPS) end
  end
  for sn=0,MAX_SONGS-1 do
    local base=SONG_BANK_BASE+sn*SONG_BANK_STRIDE
    reaper.gmem_write(base,0)
    if song_lengths[sn] then
      for i=0,SONG_HEADER_SIZE-1 do reaper.gmem_write(base+i,0) end
      local cells=song_lengths[sn]*SONG_STEP_STRIDE
      for lane=1,3 do
        local lb=base+SONG_HEADER_SIZE+(lane-1)*SONG_LANE_STRIDE
        for i=0,cells-1 do reaper.gmem_write(lb+i,0) end
      end
    end
  end

  for line in (txt.."\n"):gmatch("([^\n]*)\n") do
    local rel,count,value=line:match("^pattern_ctrl\t\t(%d+)\t(%d+)\t([^\t]+)$")
    if rel then composition_apply_run(BANK_CTRL_BASE,rel,count,value) end
    rel,count,value=line:match("^pattern_dir\t\t(%d+)\t(%d+)\t([^\t]+)$")
    if rel then composition_apply_run(BANK_DIR_BASE,rel,count,value) end
    rel,count,value=line:match("^pattern_release\t\t(%d+)\t(%d+)\t([^\t]+)$")
    if rel then composition_apply_run(BANK_RELEASE_ARP_BASE,rel,count,value) end
    rel,count,value=line:match("^midi_map\t\t(%d+)\t(%d+)\t([^\t]+)$")
    if rel then composition_apply_run(MIDI_MAP_BASE,rel,count,value) end
    rel,count,value=line:match("^pattern_data\t\t(%d+)\t(%d+)\t([^\t]+)$")
    if rel then composition_apply_run(BANK_DATA_BASE,rel,count,value) end
    local sn,r,c,v=line:match("^song_header\t(%d+)\t(%d+)\t(%d+)\t([^\t]+)$")
    if sn then composition_apply_run(SONG_BANK_BASE+clampi(tonumber(sn),0,MAX_SONGS-1)*SONG_BANK_STRIDE,r,c,v) end
    local sx,lane,rr,cc,vv=line:match("^song_lane\t(%d+),(%d+)\t(%d+)\t(%d+)\t([^\t]+)$")
    if sx then
      local base=SONG_BANK_BASE+clampi(tonumber(sx),0,MAX_SONGS-1)*SONG_BANK_STRIDE+SONG_HEADER_SIZE+(clampi(tonumber(lane),1,3)-1)*SONG_LANE_STRIDE
      composition_apply_run(base,rr,cc,vv)
    end
  end

  patterns={}; ram_pattern_loaded={}; pattern_names={}
  local loaded_bank=load_online_pattern_bank(true)
  if not loaded_bank then patterns={} end
  -- Prefer the Composition's own Pattern TXT copies for the F1 editor.
  -- They mirror the same raw bank but also restore readable names.
  local composition_patterns=join_path(parent_dir(path),"Patterns")
  if is_directory(composition_patterns) then
    load_patterns_from_dir(composition_patterns,false)
  end
  songs={}; song_names={}
  for sn=0,MAX_SONGS-1 do
    local base=SONG_BANK_BASE+sn*SONG_BANK_STRIDE
    if math.floor(reaper.gmem_read(base) or 0)==SONG_MAGIC then songs[sn]=composition_song_from_gmem(sn) end
  end
  current_pattern=clampi(tonumber(txt:match("active_pattern=(%-?%d+)")) or 0,0,MAX_PATTERNS-1)
  if not patterns[current_pattern] then
    for pi in pairs(patterns) do current_pattern=pi; break end
  end
  ensure_pattern(current_pattern)
  current_song=clampi(tonumber(txt:match("active_song=(%-?%d+)")) or 0,0,MAX_SONGS-1)
  song=songs[current_song] or new_empty_song(); song._song_index=current_song; songs[current_song]=song
  load_song_mixer_monitor_state(song)
  current_step=-1; current_cell=1; logical_cell=1
  song_step=-1; song_cell=1; song_lane=1

  -- Populate only the live edit buffers. The indexed banks above remain byte-for-byte restored.
  write_pattern_memory()
  write_one_song_memory(song,SONG_BASE)
  publish_song_mixer_state()
  save_session_state()
  if reaper.SetProjExtState then
    reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"composition_raw",txt)
    -- On load, the raw Composition is authoritative until a later Song edit
    -- or Ctrl+S updates the indexed Song project states.
    reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"song_slots_override_composition","0")
    reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"pattern_slots_override_composition","0")
  end
  return true
end

function load_song_txt(path)
  local txt=read_text_file(path)
  local loaded_song=parse_song_text(txt)
  if not loaded_song then return false end
  songs[current_song]=loaded_song
  song=loaded_song
  load_song_mixer_monitor_state(song)
  set_song_name(current_song,loaded_song.name or path)

  -- New Song packages mirror Composition: Song TXT and Patterns are siblings.
  -- Keep compatibility with the older Bank layout (root/Song/file + root/Patterns)
  -- and with old flat Song TXT files.
  local song_folder=parent_dir(path)
  local direct_patterns=join_path(song_folder,"Patterns")
  local legacy_patterns=join_path(parent_dir(song_folder),"Patterns")
  if is_directory(direct_patterns) then
    load_patterns_from_dir(direct_patterns,true)
  elseif is_directory(legacy_patterns) then
    load_patterns_from_dir(legacy_patterns,true)
  else
    load_all_patterns()
  end

  rebuild_midi_pattern_bank()
  copy_loaded_session_to_container()
  local package_ok,local_song_path=write_song_package(songs_dir(),base_name(path),txt)
  if not package_ok then return false end
  write_text_file(active_song_file_path(),norm_path(local_song_path))
  save_song_project()
  write_song_memory(false)
  return true
end
function save_song_project()
  if not reaper.SetProjExtState then return end
  save_song_mixer_monitor_state(song)
  songs[current_song]=song
  reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"active_song",tostring(current_song))
  for i=0,MAX_SONGS-1 do
    local v=songs[i]
    local key=string.format("song_%02d",i)
    if v and (i==current_song or song_has_content(v)) then
      reaper.SetProjExtState(0,PROJECT_EXT_SECTION,key,serialize_song(v))
    else
      reaper.SetProjExtState(0,PROJECT_EXT_SECTION,key,"")
    end
  end
  -- Backward-compatible active Song snapshot.
  reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"song_data",serialize_song(song))
  -- These indexed Song states are now newer than any previously saved raw
  -- Composition snapshot and must be overlaid by the Startup Loader.
  reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"song_slots_override_composition","1")
  -- Marks raw Composition snapshots created from now on as having intentional,
  -- exact Pattern Start/Play Length semantics rather than migrated Song Gate data.
  reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"pattern_window_exact_v","1")
  if reaper.MarkProjectDirty then reaper.MarkProjectDirty(0) end
end
function parse_song_text(txt)
  if not txt or not txt:find("BISCOT1_TRACKER_SONG_V1",1,true) then return nil end
  local v=new_empty_song()
  local song_end_direct=false
  local pattern_window_v=false
  local exact_pattern_windows_v=false
  txt=normalize_text_newlines(txt)
  for line in (txt.."\n"):gmatch("([^\n]*)\n") do
    if line=="song_end_direct=1" then song_end_direct=true
    elseif line:match("^song_name=") then local n=line:sub(11):gsub("^%s+",""):gsub("%s+$",""); v.name=(n~="" and n or nil)
    elseif line=="pattern_window_v=1" then pattern_window_v=true
    elseif line=="exact_pattern_windows_v=1" then exact_pattern_windows_v=true
    elseif line:match("^length=") then v.length=clampi(tonumber(line:match("=(%-?%d+)")) or 16,1,SONG_MAX_STEPS)
    elseif line:match("^lane_lengths=") then local a,b,c=line:match("=(%-?%d+),(%-?%d+),(%-?%d+)"); v.lane_lengths={clampi(a or v.length,1,SONG_MAX_STEPS),clampi(b or v.length,1,SONG_MAX_STEPS),clampi(c or v.length,1,SONG_MAX_STEPS)}
    elseif line:match("^midi_note=") then v.midi_note=clampi(tonumber(line:match("=(%-?%d+)")) or -1,-1,127)
    elseif line:match("^song_end=") then v.song_end=clampi(tonumber(line:match("=(%-?%d+)")) or -1,-1,99)
    elseif line:match("^chain_loop=") then v.chain_loop=clampi(tonumber(line:match("=(%-?%d+)")) or 0,0,1)
    elseif line:match("^song_loop_target=") then v.song_loop_target=clampi(tonumber(line:match("=(%-?%d+)")) or -1,-1,99)
    elseif line:match("^next_song_loop=") then v.chain_loop=clampi(tonumber(line:match("=(%-?%d+)")) or 0,0,1)
    elseif line:match("^lane_repeat=") then local a,b,c=line:match("=(%-?%d+),(%-?%d+),(%-?%d+)"); v.lane_repeat={clampi(a or 1,1,1000),clampi(b or 1,1,1000),clampi(c or 1,1,1000)}
    elseif line:match("^mixer_volumes=") then local a,b,c=line:match("=(%-?%d+),(%-?%d+),(%-?%d+)"); v.mixer_volumes={clampi(a or 100,0,100),clampi(b or 100,0,100),clampi(c or 100,0,100)}
    elseif line:match("^mixer_mute_mask=") then v.mixer_mute_mask=clampi(tonumber(line:match("=(%-?%d+)")) or 0,0,7)
    elseif line:match("^mixer_solo_lane=") then v.mixer_solo_lane=clampi(tonumber(line:match("=(%-?%d+)")) or 0,0,3)
    elseif line:match("^mode=") then local old=clampi(tonumber(line:match("=(%-?%d+)")) or 0,0,16); v.song_end=(old==1) and -1 or -1; local rc=old>=2 and old or 1; v.lane_repeat={rc,rc,rc}
    elseif line:match("^lane_mode=") then local a,b,c=line:match("=(%-?%d+),(%-?%d+),(%-?%d+)"); local function oldrc(x) x=clampi(x or 0,0,16); return x>=2 and x or 1 end; v.lane_repeat={oldrc(a),oldrc(b),oldrc(c)}
    elseif line:match("^lane_speed\t") then local la,ri,sp=line:match("^lane_speed\t([^\t]+)\t([^\t]+)\t([^\t]+)"); la=clampi(la or 1,1,3); ri=clampi(ri or 1,1,1000); v.lane_speeds[la][ri]=clamp(sp or 1,0.25,16)
    elseif line:match("^filter_owner=") then v.filter_owner=clampi(tonumber(line:match("=(%-?%d+)")) or 1,1,3)
    elseif line:match("^filter_mask=") then v.filter_mask=clampi(tonumber(line:match("=(%-?%d+)")) or 7,0,7)
    elseif line:match("^lfo_owner=") then local a,b,c=line:match("=(%-?%d+),(%-?%d+),(%-?%d+)"); v.lfo_owner={clampi(a or 1,1,3),clampi(b or 1,1,3),clampi(c or 1,1,3)}
    elseif line:match("^lane\t") then
      local la,st,pa,sp,tr,rp,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22=line:match("^lane\t([^\t]+)\t([^\t]+)\t([^\t]+)\t([^\t]+)\t([^\t]+)\t([^\t]+)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)")
      local is_new=pattern_window_v
      local ps,pl,le,ma,fo,fm,l1,l2,l3,scs,sca,ds,df,dv,dp,so
      if is_new then ps,pl,le,ma,fo,fm,l1,l2,l3,scs,sca,ds,df,dv,dp,so=a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22
      else
        -- Legacy Song Gate was an old percentage-duration field. It must not be
        -- converted into Pattern Play Length: values around 0.97 turned a
        -- 64-step Pattern into 62 steps and silently removed steps 62 and 63.
        -- The obsolete Gate is ignored and the complete Pattern is restored.
        ps=0; local pidx=clampi(pa or -1,-1,999); local plen=(pidx>=0 and ensure_pattern(pidx).length) or 1; pl=math.max(1,plen); le,ma,fo,fm,l1,l2,l3,scs,sca,ds,df,dv,dp,so=a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,0
      end
      la=clampi(la or 1,1,3); st=clampi(st or 0,0,SONG_MAX_STEPS-1)
      local mask=tonumber(ma); if mask==nil then mask=63 end
      local present={}
      for i=1,22 do present[i]=(math.floor(mask/(2^(i-1)))%2)==1 end
      v.lanes[la][st]={pattern=clampi(pa or -1,-1,999),speed=clamp(sp or 1,0.25,16),transpose=clampi(tr or 0,-96,96),repeat_count=clampi(rp or 1,1,16),pattern_start=clampi(ps or 0,0,MAX_STEPS-1),play_length=clampi(pl or 1,1,MAX_STEPS),legato=clampi(tonumber(le) or 0,0,1),filter_owner=clampi(fo or 1,1,3),filter_mask=clampi(fm or 7,0,7),lfo1_owner=clampi(l1 or 1,1,3),lfo2_owner=clampi(l2 or 1,1,3),lfo3_owner=clampi(l3 or 1,1,3),sidechain_source=clampi(scs or 0,0,3),sidechain_amount=clampi(sca or 0,0,100),delay_speed=clamp(ds or 0,0,8),delay_feedback=clampi(df or 0,0,100),delay_volume=clampi(dv or 100,0,100),delay_pingpong_depth=clampi(dp or 0,0,100),start_offset=clampi(so or 0,0,31),_present=present}
    end
  end
  -- 0.157 one-time repair for Songs already converted by 0.156 or earlier.
  -- Those files already say pattern_window_v=1, but their old Gate may have
  -- been frozen as Play Length = Pattern Length - 2. New files carry the exact
  -- marker, so an intentional two-step-short window remains untouched.
  if not exact_pattern_windows_v then
    for lane=1,3 do
      for _,t in pairs(v.lanes[lane] or {}) do
        local pidx=clampi((t and t.pattern) or -1,-1,999)
        if pidx>=0 then
          local plen=clampi(ensure_pattern(pidx).length or 1,1,MAX_STEPS)
          local pst=clampi(t.pattern_start or 0,0,plen-1)
          local pl=clampi(t.play_length or 0,0,MAX_STEPS)
          if pst==0 and plen>=3 and pl==plen-2 then t.play_length=plen
          elseif pl<=0 then t.play_length=math.max(1,plen-pst) end
        end
      end
    end
  end
  v._exact_pattern_windows=true
  if not song_end_direct then
    -- Packages before direct Song End used 0=Loop and 1..100=Song 0..99.
    -- Preserve existing chains while removing the obsolete Loop value.
    v.song_end=clampi((v.song_end or 0)-1,-1,99)
  end
  ensure_song_lane_meta(v)
  ensure_song_mixer(v)
  return v
end
function load_song_project()
  if not reaper.GetProjExtState then return false end
  songs={}
  local loaded=false
  for i=0,MAX_SONGS-1 do
    local _,txt=reaper.GetProjExtState(0,PROJECT_EXT_SECTION,string.format("song_%02d",i))
    local v=parse_song_text(txt)
    if v then songs[i]=v; if v.name and v.name~="" then song_names[i]=v.name end; loaded=true end
  end
  local _,active=reaper.GetProjExtState(0,PROJECT_EXT_SECTION,"active_song")
  current_song=clampi(tonumber(active) or 0,0,MAX_SONGS-1)
  if not loaded then
    local _,txt=reaper.GetProjExtState(0,PROJECT_EXT_SECTION,"song_data")
    local v=parse_song_text(txt)
    if v then songs[current_song]=v; if v.name and v.name~="" then song_names[current_song]=v.name end; loaded=true end
  end
  set_active_song(current_song)
  return loaded
end
function push_live_delay_state()
  if not song_playing or song_step<0 or song_step>=song_lane_length(song_lane) then return end
  local t=song.lanes[song_lane][song_step]
  if not t then return end
  local present=t._present or {}
  if not (present[17] or present[18] or present[19] or present[20]) then return end
  reaper.gmem_attach(GMEM_NS)
  local b=DELAY_LIVE_BASE+(song_lane-1)*DELAY_LIVE_STRIDE
  if present[17] then reaper.gmem_write(b+0,clamp(t.delay_speed or 0,0,8)) end
  if present[18] then reaper.gmem_write(b+1,clampi(t.delay_feedback or 0,0,100)) end
  if present[19] then reaper.gmem_write(b+2,clampi(t.delay_volume or 100,0,100)) end
  if present[20] then reaper.gmem_write(b+3,clampi(t.delay_pingpong_depth or 0,0,100)) end
  reaper.gmem_write(b+4,math.floor(reaper.gmem_read(b+4) or 0)+1)
end

function push_live_sidechain_state()
  if not song_playing or song_step<0 or song_step>=song_lane_length(song_lane) then return end
  local t=song.lanes[song_lane][song_step]
  if not t then return end
  local present=t._present or {}
  if not (present[15] or present[16]) then return end
  reaper.gmem_attach(GMEM_NS)
  local b=SIDECHAIN_LIVE_BASE+(song_lane-1)*SIDECHAIN_LIVE_STRIDE
  local source=present[15] and clampi(t.sidechain_source or 0,0,3) or clampi(reaper.gmem_read(b+0) or 0,0,3)
  local amount=present[16] and clampi(t.sidechain_amount or 0,0,100) or clampi(reaper.gmem_read(b+1) or 0,0,100)
  if (present[15] and source<=0) or (present[16] and amount<=0) then source=0; amount=0 end
  reaper.gmem_write(b+0,source)
  reaper.gmem_write(b+1,amount)
  reaper.gmem_write(b+2,math.floor(reaper.gmem_read(b+2) or 0)+1)
end

function refresh_song_after_edit()
  biscot_record_song_undo_after_edit()
  save_song_project()
  if song_playing then
    -- Live edit: update the active Song data in place without issuing a new
    -- transport command. Writing LUA_BASE+3=0 or changing the serial here
    -- would stop/restart playback every time a value is changed.
    reaper.gmem_attach(GMEM_NS)
    write_one_song_memory(song,SONG_BASE)
    write_one_song_memory(song,SONG_BANK_BASE+current_song*SONG_BANK_STRIDE)
  rebuild_song_midi_map()
    push_live_delay_state()
    push_live_sidechain_state()
  else
    write_song_memory(false)
  end
  -- Do not rewrite all 100 Song slots after every parameter edit.
end

function preview_song_row_pattern()
  -- Never interrupt or restart the Song transport with navigation previews.
  if song_playing then return end
  if song_step<0 or song_step>=song_lane_length(song_lane) then return end
  local t=song.lanes[song_lane][song_step]
  if not t or (t.pattern or -1)<0 then return end
  local target=clampi(t.pattern,0,MAX_PATTERNS-1)
  local pat=ensure_pattern(target)
  local plen=clampi(pat.length or 16,1,MAX_STEPS)
  local _,window_start,window_len=song_pattern_window(t)
  local window_end=math.min(plen-1,window_start+window_len-1)
  local start_step=window_start
  local has_content=false
  for st=window_start,window_end do
    if #(pat.steps[st][1] or {})>0 then has_content=true break end
  end
  if not has_content then return end

  reaper.gmem_attach(GMEM_NS)
  lua_serial=lua_serial+1
  reaper.gmem_write(LUA_BASE+0,LUA_MAGIC)
  reaper.gmem_write(LUA_BASE+2,1)
  reaper.gmem_write(LUA_BASE+3,0)
  reaper.gmem_write(LUA_BASE+1,lua_serial)

  local preview_len=math.min(3,window_len)
  for sidx=0,MAX_STEPS-1 do
    local sb=step_base(sidx)
    if sidx<preview_len then
      local src=pat.steps[start_step+sidx]
      local flat={}
      -- F3 navigation preview is operator-only and uses the selected Song lane.
      for _,c in ipairs(src[1] or {}) do
        if #flat<MAX_CELLS then
          local rid=c.id
          local pd=PARAM_BY_ID[c.id]
          if pd and pd.map then rid=1000+pd.map[song_lane] end
          local value=tonumber(c.value) or 0
          if c.id==1 then value=clampi(value+(t.transpose or 0),0,127) end
          flat[#flat+1]={id=rid,value=value}
        end
      end
      reaper.gmem_write(sb,#flat)
      for i=1,MAX_CELLS do
        local cb=sb+1+(i-1)*CELL_SIZE
        local c=flat[i]
        reaper.gmem_write(cb,c and c.id or 0)
        reaper.gmem_write(cb+1,c and c.value or 0)
      end
    else
      reaper.gmem_write(sb,0)
    end
  end
  reaper.gmem_write(LUA_BASE+4,preview_len)
  reaper.gmem_write(LUA_BASE+5,1)
  reaper.gmem_write(LUA_BASE+6,clamp(t.speed or 1,0.25,16))
  reaper.gmem_write(LUA_BASE+7,song_lane)
  reaper.gmem_write(LUA_BASE+8,1)
  reaper.gmem_write(REPEAT_TRANSPOSE_BASE,0)
  reaper.gmem_write(RELEASE_ARP_CURRENT,0)
  lua_serial=lua_serial+1
  reaper.gmem_write(PLAY_STATUS,1)
  reaper.gmem_write(LUA_BASE+3,1)
  reaper.gmem_write(LUA_BASE+1,lua_serial)
end
function song_value_bounds()
  if song_lane==4 then return -1,99
  elseif song_is_lane_meta_row() then
    if song_cell==1 then return 1,SONG_MAX_STEPS end
    if song_cell==2 then return -1,127 end
    return 0,0
  elseif song_step==song_lane_length(song_lane) then return 1,1000
  elseif is_song_speed_row() then return 0.25,16
  else
    local idx=song_current_param_index()
    local t=song.lanes[song_lane][song_step]
    if idx==1 then return -1,999 elseif idx==2 then return 0.25,16 elseif idx==3 then return -96,96 elseif idx==4 then return 1,16
    elseif idx==5 then local plen=song_pattern_window(t); return 0,math.max(0,plen-1)
    elseif idx==6 then local plen,start=song_pattern_window(t); return 1,math.max(1,plen-start)
    elseif idx==7 or idx==8 or idx==21 then return 0,0 elseif idx==22 then return 0,31 elseif idx==9 then return 0,1 elseif idx==11 then return 0,7 elseif idx==15 then return 0,3 elseif idx==16 then return 0,100 elseif idx==17 then return 0,8 elseif idx>=18 then return 0,100 else return 1,3 end
  end
end
function change_song_value(delta,big)
  local mult=jump_sizes[jump_index]*(big and 5 or 1)
  local changed=false
  ensure_song_lane_meta(song)
  if song_lane==4 then
    local old=clampi(song.song_end or -1,-1,99); local nv=clampi(old+delta*mult,-1,99)
    if nv~=old then song.song_end=nv; changed=true end
  elseif song_is_lane_meta_row() then
    if song_cell==1 then
      local old=song_lane_length(song_lane)
      local nv=clampi(old+delta*mult,1,SONG_MAX_STEPS)
      if nv~=old then song.lane_lengths[song_lane]=nv; song_recompute_length(song); changed=true end
    elseif song_cell==2 then
      local old=clampi(song.midi_note or -1,-1,127)
      local nv=clampi(old+delta*mult,-1,127)
      if nv~=old then song.midi_note=nv; rebuild_song_midi_map(); changed=true end
    else return end
  elseif song_step==song_lane_length(song_lane) then
    local old=song_lane_repeat_count(song_lane)
    local nv=clampi(old+delta*mult,1,1000)
    if nv~=old then
      song.lane_repeat[song_lane]=nv
      for i=old+1,nv do song.lane_speeds[song_lane][i]=song.lane_speeds[song_lane][i-1] or 1 end
      changed=true
    end
  elseif is_song_speed_row() then
    local idx=song_speed_index(); local old=song.lane_speeds[song_lane][idx] or 1
    local nv=clamp(old+delta*0.25*mult,0.25,16)
    if math.abs(nv-old)>1e-12 then song.lane_speeds[song_lane][idx]=nv; changed=true end
  else
    local t=song.lanes[song_lane][song_step]
    local idx=song_current_param_index()
    if not t or not idx then return end
    if idx==1 then
      local old=t.pattern or -1; local nv=clampi(old+delta*mult,-1,999)
      if nv~=old then
        t.pattern=nv
        if nv>=0 then local plen=clampi(ensure_pattern(nv).length or 1,1,MAX_STEPS); t.pattern_start=0; t.play_length=plen else t.pattern_start=0; t.play_length=1 end
        changed=true
      end
    elseif idx==2 then
      local old=t.speed or 1; local nv=clamp(old+delta*0.25*mult,0.25,16)
      if math.abs(nv-old)>1e-12 then t.speed=nv; changed=true end
    elseif idx==3 then
      local old=t.transpose or 0; local nv=clampi(old+delta*mult,-96,96)
      if nv~=old then t.transpose=nv; changed=true end
    elseif idx==4 then
      local old=t.repeat_count or 1; local nv=clampi(old+delta*mult,1,16)
      if nv~=old then t.repeat_count=nv; changed=true end
    elseif idx==5 then local plen,old,play=song_pattern_window(t); local nv=clampi(old+delta*mult,0,plen-1); if nv~=old then t.pattern_start=nv; t.play_length=math.min(play,math.max(1,plen-nv)); changed=true end
    elseif idx==6 then local plen,start,old=song_pattern_window(t); local nv=clampi(old+delta*mult,1,math.max(1,plen-start)); if nv~=old then t.play_length=nv; changed=true end
    elseif idx==7 or idx==8 or idx==21 then return
    elseif idx==9 then local old=t.legato or 0; local nv=clampi(old+delta*mult,0,1); if nv~=old then t.legato=nv; changed=true end
    elseif idx==10 then local old=t.filter_owner or 1; local nv=clampi(old+delta*mult,1,3); if nv~=old then t.filter_owner=nv; changed=true end
    elseif idx==11 then local old=t.filter_mask or 7; local nv=clampi(old+delta*mult,0,7); if nv~=old then t.filter_mask=nv; changed=true end
    elseif idx==12 then local old=t.lfo1_owner or 1; local nv=clampi(old+delta*mult,1,3); if nv~=old then t.lfo1_owner=nv; changed=true end
    elseif idx==13 then local old=t.lfo2_owner or 1; local nv=clampi(old+delta*mult,1,3); if nv~=old then t.lfo2_owner=nv; changed=true end
    elseif idx==14 then local old=t.lfo3_owner or 1; local nv=clampi(old+delta*mult,1,3); if nv~=old then t.lfo3_owner=nv; changed=true end
    elseif idx==15 then local old=t.sidechain_source or 0; local nv=clampi(old+delta*mult,0,3); if nv~=old then t.sidechain_source=nv; changed=true end
    elseif idx==16 then local old=t.sidechain_amount or 0; local nv=clampi(old+delta*mult,0,100); if nv~=old then t.sidechain_amount=nv; changed=true end
    elseif idx==17 then local old=t.delay_speed or 0; local nv=clamp(old+delta*0.125*mult,0,8); nv=math.floor(nv*8+0.5)/8; if math.abs(nv-old)>1e-12 then t.delay_speed=nv; changed=true end
    elseif idx==18 then local old=t.delay_feedback or 0; local nv=clampi(old+delta*mult,0,99); if nv~=old then t.delay_feedback=nv; changed=true end
    elseif idx==19 then local old=t.delay_volume or 100; local nv=clampi(old+delta*mult,0,99); if nv~=old then t.delay_volume=nv; changed=true end
    elseif idx==20 then local old=t.delay_pingpong_depth or 0; local nv=clampi(old+delta*mult,0,99); if nv~=old then t.delay_pingpong_depth=nv; changed=true end
    elseif idx==22 then local old=t.start_offset or 0; local nv=clampi(old+delta*mult,0,31); if nv~=old then t.start_offset=nv; changed=true end
    end
  end
  if changed then
    refresh_song_after_edit()
    speak_song_value_only()
    if song_step>=0 and song_step<song_lane_length(song_lane) then preview_song_row_pattern() end
  end
end
function set_song_value_extreme(to_max)
  local lo,hi=song_value_bounds(); local target=to_max and hi or lo; local changed=false
  ensure_song_lane_meta(song)
  if song_lane==4 then
    target=clampi(target,-1,99); if (song.song_end or -1)~=target then song.song_end=target; changed=true end
  elseif song_is_lane_meta_row() then
    if song_cell==1 then
      target=clampi(target,1,SONG_MAX_STEPS); if song_lane_length(song_lane)~=target then song.lane_lengths[song_lane]=target; song_recompute_length(song); changed=true end
    elseif song_cell==2 then
      target=clampi(target,-1,127); if (song.midi_note or -1)~=target then song.midi_note=target; rebuild_song_midi_map(); changed=true end
    else return end
  elseif song_step==song_lane_length(song_lane) then
    target=clampi(target,1,1000); if song_lane_repeat_count(song_lane)~=target then local old=song_lane_repeat_count(song_lane); song.lane_repeat[song_lane]=target; for i=old+1,target do song.lane_speeds[song_lane][i]=song.lane_speeds[song_lane][i-1] or 1 end; changed=true end
  elseif is_song_speed_row() then
    local idx=song_speed_index(); target=clamp(target,0.25,16); if math.abs((song.lane_speeds[song_lane][idx] or 1)-target)>1e-12 then song.lane_speeds[song_lane][idx]=target; changed=true end
  else
    local t=song.lanes[song_lane][song_step]; local idx=song_current_param_index()
    if not t or not idx or idx==7 or idx==8 or idx==21 then return end
    if idx==1 then if (t.pattern or -1)~=target then
      t.pattern=target
      if target>=0 then local plen=clampi(ensure_pattern(target).length or 1,1,MAX_STEPS); t.pattern_start=0; t.play_length=plen else t.pattern_start=0; t.play_length=1 end
      changed=true
    end
    elseif idx==2 then if math.abs((t.speed or 1)-target)>1e-12 then t.speed=target; changed=true end
    elseif idx==3 then if (t.transpose or 0)~=target then t.transpose=target; changed=true end
    elseif idx==4 then if (t.repeat_count or 1)~=target then t.repeat_count=target; changed=true end
    elseif idx==5 then local plen=song_pattern_window(t); target=clampi(target,0,plen-1); if (t.pattern_start or 0)~=target then t.pattern_start=target; local _,_,pl=song_pattern_window(t); t.play_length=pl; changed=true end
    elseif idx==6 then local plen,start=song_pattern_window(t); target=clampi(target,1,math.max(1,plen-start)); if (t.play_length or 0)~=target then t.play_length=target; changed=true end
    elseif idx==9 then if (t.legato or 0)~=target then t.legato=target; changed=true end
    elseif idx==10 then if (t.filter_owner or 1)~=target then t.filter_owner=target; changed=true end
    elseif idx==11 then if (t.filter_mask or 7)~=target then t.filter_mask=target; changed=true end
    elseif idx==12 then if (t.lfo1_owner or 1)~=target then t.lfo1_owner=target; changed=true end
    elseif idx==13 then if (t.lfo2_owner or 1)~=target then t.lfo2_owner=target; changed=true end
    elseif idx==14 then if (t.lfo3_owner or 1)~=target then t.lfo3_owner=target; changed=true end
    elseif idx==15 then target=clampi(target,0,3); if (t.sidechain_source or 0)~=target then t.sidechain_source=target; changed=true end
    elseif idx==16 then target=clampi(target,0,100); if (t.sidechain_amount or 0)~=target then t.sidechain_amount=target; changed=true end
    elseif idx==17 then target=clamp(target,0,8); if math.abs((t.delay_speed or 0)-target)>1e-12 then t.delay_speed=target; changed=true end
    elseif idx==18 then target=clampi(target,0,100); if (t.delay_feedback or 0)~=target then t.delay_feedback=target; changed=true end
    elseif idx==19 then target=clampi(target,0,100); if (t.delay_volume or 100)~=target then t.delay_volume=target; changed=true end
    elseif idx==20 then target=clampi(target,0,100); if (t.delay_pingpong_depth or 0)~=target then t.delay_pingpong_depth=target; changed=true end
    elseif idx==22 then target=clampi(target,0,31); if (t.start_offset or 0)~=target then t.start_offset=target; changed=true end end
  end
  if changed then refresh_song_after_edit(); speak_song_value_only(); if song_step>=0 and song_step<song_lane_length(song_lane) then preview_song_row_pattern() end end
end
function delete_song_parameter()
  if song_is_lane_meta_row() and song_cell==2 then
    if (song.midi_note or -1)~=-1 then
      song.midi_note=-1
      rebuild_song_midi_map()
      refresh_song_after_edit()
    end
    speak("MIDI Note, Off")
    return
  end
  if song_step<0 or song_step>=song_lane_length(song_lane) then return end
  local t=song.lanes[song_lane][song_step]
  local idx=song_current_param_index()
  if not t or not idx or (SONG_PARAM_LIST[idx] and SONG_PARAM_LIST[idx].info) then return end
  t._present=t._present or {}
  local group=song_parameter_group_id(idx)
  if group==1 then
    t._present[1]=nil
    t._present[5]=nil
    t._present[6]=nil
    t.pattern=-1
    t.pattern_start=0
    t.play_length=0
  else
    t._present[idx]=nil
    t[SONG_KEYS[idx]]=SONG_DEFAULTS[idx]
  end
  if t._order then for i=#t._order,1,-1 do if t._order[i]==group then table.remove(t._order,i) end end end
  local vis=song_visible_params(t)
  if #vis==0 then
    song.lanes[song_lane][song_step]=nil
    song_cell=1
    refresh_song_after_edit()
    speak("step "..song_step..". empty")
  else
    song_cell=clampi(song_cell,1,#vis)
    refresh_song_after_edit()
    speak_song_position()
  end
end
function clear_song_step()
  if song_step<0 or song_step>=song_lane_length(song_lane) then return end
  if song.lanes[song_lane][song_step] then song.lanes[song_lane][song_step]=nil; refresh_song_after_edit(); speak("step "..song_step.." cleared") end
end
function clear_song_lane()
  local had=next(song.lanes[song_lane])~=nil
  if had then song.lanes[song_lane]={}; refresh_song_after_edit(); speak("Operator "..song_lane.." cleared") end
end
function reset_current_song()
  songs[current_song]=new_empty_song(); song=songs[current_song]
  load_song_mixer_monitor_state(song)
  set_song_name(current_song,"")
  song_step=-1; song_cell=1; song_lane=1
  refresh_song_after_edit(); speak("Song "..current_song.." initialized")
end

function clear_pattern_step_selection()
  pattern_step_selection_anchor=nil
  pattern_step_selection_end=nil
  pattern_selection_last_shift_time=0
  pattern_selection_last_shift_key=0
end

function pattern_step_selection_active()
  return pattern_step_selection_anchor~=nil and pattern_step_selection_end~=nil
end

function pattern_selection_bounds()
  if not pattern_step_selection_active() then return nil,nil end
  return math.min(pattern_step_selection_anchor,pattern_step_selection_end),
         math.max(pattern_step_selection_anchor,pattern_step_selection_end)
end

function extend_pattern_step_selection(delta)
  -- F1 must always enter a real Pattern step before starting the selection.
  -- This also makes Shift+Up/Down work immediately after opening F1 on Length,
  -- Repeat or one of the Speed/Transpose rows.
  if not is_real_step() then
    local len=pattern_length()
    if is_header_row() then
      current_step=clampi(last_real_step or 0,0,len-1)
    elseif current_step>=len then
      current_step=len-1
    else
      current_step=0
    end
    last_real_step=current_step
    logical_cell=1
    current_cell=1
  end
  pattern_selection_last_shift_time=reaper.time_precise and reaper.time_precise() or 0
  pattern_selection_last_shift_key=delta
  if not pattern_step_selection_active() then
    -- Come nella Song: il primo Shift+freccia seleziona lo step corrente senza muoversi.
    pattern_step_selection_anchor=current_step
    pattern_step_selection_end=current_step
  else
    local next_step=clampi(pattern_step_selection_end+delta,0,pattern_length()-1)
    if next_step==pattern_step_selection_end then return end -- silent selection border
    pattern_step_selection_end=next_step
    current_step=pattern_step_selection_end
    logical_cell=1
    current_cell=1
    last_real_step=current_step
  end
  local a,b=pattern_selection_bounds()
  local prefix=(a==b) and ("Step "..a.." selected") or ("Steps "..a.." to "..b.." selected")
  local c=current_cell_obj()
  if c then
    local p=PARAM_BY_ID[c.id]
    local name
    if c.id==3 then name="Previous value"
    elseif c.id==P_PATTERN_LENGTH then name="Length"
    elseif c.id==P_PATTERN_MIDI_NOTE then name="MIDI Note"
    elseif c.id==P_STEP_RECORDING then name="Step Recording"
    elseif c.id==P_RELEASE_ARP then name="Release Arpeggio"
    elseif c.id==P_PATTERN_REPEAT then name="Repeat"
    elseif c.id==P_PATTERN_SPEED then name="Speed"
    elseif c.id==P_PATTERN_TRANSPOSE then name="Transpose"
    else name=p and p.name or "Parameter" end
    speak(prefix..". "..name..", "..value_text(c))
  else
    speak(prefix..". empty")
  end
  -- Shift+Up/Down in F1 auditions the selected step exactly like normal arrows.
  preview_pattern_step(current_step)
end

function pattern_plain_arrow_is_shift_echo(delta)
  if not pattern_step_selection_active() then return false end
  local now=reaper.time_precise and reaper.time_precise() or 0
  return pattern_selection_last_shift_key==delta and (now-pattern_selection_last_shift_time)<0.20
end

function clone_pattern_step_data(step)
  local out={{},{},{}}
  for tab=1,3 do
    for _,c in ipairs(step[tab] or {}) do
      out[tab][#out[tab]+1]={id=c.id,value=c.value}
    end
  end
  return out
end

function copy_entire_current_pattern()
  -- Copy exactly the Pattern bank that the Song/Tracker is playing.  The Lua
  -- editor cache can occasionally be empty while the already loaded GMEM bank
  -- is still valid; reading it here is deliberately read-only and must never
  -- replace patterns[], rebuild the bank or touch Song/Core state.
  reaper.gmem_attach(GMEM_NS)
  local txt=composition_pattern_text_from_gmem(current_pattern) or biscot_serialize_pattern(current_pattern)
  whole_pattern_clipboard=txt
  whole_pattern_clipboard_source=current_pattern
  pattern_clipboard_mode="full_pattern"
  clipboard_enter_pending=false
  clear_pattern_step_selection()
  if biscot_copy_text_to_clipboard(txt) then
    speak("Pattern "..tostring(current_pattern).." text copied")
  else
    speak("Pattern "..tostring(current_pattern).." copied internally")
  end
  return true
end

function paste_entire_pattern()
  local txt=biscot_get_text_from_clipboard()
  if not (txt and txt:find("BISCOT1_TRACKER_RAM_V2",1,true)) then txt=whole_pattern_clipboard end
  if not txt then return false end
  local restored=biscot_parse_pattern_snapshot(txt,current_pattern)
  if not restored then speak("Pattern paste failed") return false end
  patterns[current_pattern]=restored
  current_step=0
  current_tab=1
  current_cell=1
  logical_cell=1
  last_real_step=0
  clear_pattern_step_selection()
  refresh_pattern_after_edit()
  speak("Pattern pasted")
  return true
end

function copy_selected_pattern_steps()
  local a,b=pattern_selection_bounds()
  if not a then return false end
  local pat=ensure_pattern(current_pattern)
  pattern_steps_clipboard={}
  pattern_clipboard_mode="steps"
  for st=a,b do
    pattern_steps_clipboard[#pattern_steps_clipboard+1]=clone_pattern_step_data(pat.steps[st])
  end
  clipboard_enter_pending=false
  local n=b-a+1
  speak(n==1 and "One step copied" or (n.." steps copied"))
  return true
end

function paste_selected_pattern_steps()
  if not pattern_steps_clipboard or not is_real_step() then return false end
  local pat=ensure_pattern(current_pattern)
  local copied=0
  for i,step in ipairs(pattern_steps_clipboard) do
    local dst=current_step+i-1
    if dst>=pattern_length() then break end
    pat.steps[dst]=clone_pattern_step_data(step)
    copied=copied+1
  end
  if copied<=0 then return false end
  refresh_pattern_after_edit()
  clear_pattern_step_selection()
  logical_cell=1
  current_cell=1
  speak(copied==1 and "One step pasted" or (copied.." steps pasted"))
  return true
end

function cut_selected_pattern_steps()
  local a,b=pattern_selection_bounds()
  if not a then return false end
  if not copy_selected_pattern_steps() then return false end
  local pat=ensure_pattern(current_pattern)
  for st=a,b do pat.steps[st]={{},{},{}} end
  refresh_pattern_after_edit()
  clear_pattern_step_selection()
  logical_cell=1; current_cell=1
  local n=b-a+1
  speak(n==1 and "One step cut" or (n.." steps cut"))
  return true
end

function delete_selected_pattern_steps()
  local a,b=pattern_selection_bounds()
  if not a then return false end
  local pat=ensure_pattern(current_pattern)
  for st=a,b do pat.steps[st]={{},{},{}} end
  refresh_pattern_after_edit()
  clear_pattern_step_selection()
  logical_cell=1; current_cell=1
  local n=b-a+1
  speak(n==1 and "One step deleted" or (n.." steps deleted"))
  return true
end

function clear_song_step_selection()
  song_step_selection_anchor=nil
  song_step_selection_end=nil
  song_selection_last_shift_time=0
  song_selection_last_shift_key=0
end

function song_step_selection_active()
  return song_step_selection_anchor~=nil and song_step_selection_end~=nil
end

function song_selection_bounds()
  if not song_step_selection_active() then return nil,nil end
  return math.min(song_step_selection_anchor,song_step_selection_end),
         math.max(song_step_selection_anchor,song_step_selection_end)
end

function extend_song_step_selection(delta)
  if song_step<0 or song_step>=song_lane_length(song_lane) then return end
  song_selection_last_shift_time=reaper.time_precise and reaper.time_precise() or 0
  song_selection_last_shift_key=delta
  if not song_step_selection_active() then
    -- First Shift+arrow selects the current row and deliberately does not move.
    song_step_selection_anchor=song_step
    song_step_selection_end=song_step
  else
    local preferred_param,preferred_cell=song_cursor_preference()
    local next_step=clampi(song_step_selection_end+delta,0,song_lane_length(song_lane)-1)
    if next_step==song_step_selection_end then return end -- silent selection border
    song_step_selection_end=next_step
    song_step=song_step_selection_end
    restore_song_cursor_preference(preferred_param,preferred_cell)
  end
  local a,b=song_selection_bounds()
  local prefix=(a==b) and ("Step "..a.." selected") or ("Steps "..a.." to "..b.." selected")
  local t=song.lanes[song_lane][song_step]
  local idx=song_current_param_index()
  if t and idx then
    speak(prefix..". "..SONG_PARAM_LIST[idx].name..", "..song_value_text(idx,t))
  else
    speak(prefix..". empty")
  end
  preview_song_row_pattern()
end

function song_plain_arrow_is_shift_echo(delta)
  if not song_step_selection_active() then return false end
  local now=reaper.time_precise and reaper.time_precise() or 0
  return song_selection_last_shift_key==delta and (now-song_selection_last_shift_time)<0.20
end

function clone_song_step_data(t)
  if not t then return {_empty=true} end
  local c={_present={},_order={}}
  for _,id in ipairs(t._order or {}) do c._order[#c._order+1]=id end
  for i=1,#SONG_PARAM_LIST do
    local key=SONG_KEYS[i]
    if key then c[key]=t[key] end
    if t._present and t._present[i] then c._present[i]=true end
  end
  return c
end

function copy_entire_current_song()
  save_song_mixer_monitor_state(song)
  local txt=serialize_song(song)
  whole_song_clipboard=txt
  whole_song_clipboard_source=current_song
  song_clipboard_mode="full_song"
  song_steps_enter_pending=false
  song_clipboard_enter_pending=false
  clear_song_step_selection()
  if biscot_copy_text_to_clipboard(txt) then
    speak("Song "..tostring(current_song).." text copied")
  else
    speak("Song "..tostring(current_song).." copied internally")
  end
  return true
end

function paste_entire_song()
  local txt=biscot_get_text_from_clipboard()
  if not (txt and txt:find("BISCOT1_TRACKER_SONG_V1",1,true)) then txt=whole_song_clipboard end
  if not txt then return false end
  local restored=parse_song_text(txt)
  if not restored then speak("Song paste failed") return false end

  if song_playing then
    song_playing=false
    song_play_start_time=-1000
    write_song_memory(false)
  end

  -- The destination Song keeps its own user-facing name, just as a Pattern
  -- paste keeps the destination Pattern number instead of becoming the source.
  local destination_name=song_names[current_song] or song.name
  restored.name=(destination_name and destination_name~="") and destination_name or nil
  restored._song_index=current_song
  songs[current_song]=restored
  song=restored
  if restored.name then song_names[current_song]=restored.name else song_names[current_song]=nil end
  load_song_mixer_monitor_state(song)

  song_lane=1
  song_last_operator_lane=1
  song_mixer_lane=1
  song_step=0
  song_cell=1
  song_logical_param=nil
  song_logical_cell=1
  clear_song_step_selection()
  song_steps_enter_pending=false
  song_clipboard_enter_pending=false
  refresh_song_after_edit()
  update_controller_title()
  speak("Song pasted")
  return true
end

function copy_selected_song_steps()
  local a,b=song_selection_bounds()
  if not a then return false end
  song_steps_clipboard={}
  song_steps_clipboard_lane=song_lane
  song_steps_clipboard_song=current_song
  song_steps_clipboard_source_end=b
  song_clipboard_mode="steps"
  song_steps_enter_pending=true
  song_clipboard_enter_pending=false
  for st=a,b do
    song_steps_clipboard[#song_steps_clipboard+1]=clone_song_step_data(song.lanes[song_lane][st])
  end
  local n=b-a+1
  speak(n==1 and "One step copied" or (n.." steps copied"))
  return true
end

function paste_selected_song_steps()
  if not song_steps_clipboard or song_step<0 or song_step>=song_lane_length(song_lane) then return false end
  local copied=0
  for i,t in ipairs(song_steps_clipboard) do
    local dst=song_step+i-1
    if dst>=song_lane_length(song_lane) then break end
    if t and t._empty then
      song.lanes[song_lane][dst]=nil
    else
      song.lanes[song_lane][dst]=clone_song_step_data(t)
    end
    copied=copied+1
  end
  if copied<=0 then return false end
  refresh_song_after_edit()
  clear_song_step_selection()
  speak(copied==1 and "One step pasted" or (copied.." steps pasted"))
  return true
end

function cut_selected_song_steps()
  local a,b=song_selection_bounds()
  if not a then return false end
  if not copy_selected_song_steps() then return false end
  song_steps_enter_pending=false
  for st=a,b do song.lanes[song_lane][st]=nil end
  refresh_song_after_edit()
  clear_song_step_selection()
  local n=b-a+1
  speak(n==1 and "One step cut" or (n.." steps cut"))
  return true
end

function delete_selected_song_steps()
  local a,b=song_selection_bounds()
  if not a then return false end
  for st=a,b do song.lanes[song_lane][st]=nil end
  refresh_song_after_edit()
  clear_song_step_selection()
  song_cell=1
  local n=b-a+1
  speak(n==1 and "One step deleted" or (n.." steps deleted"))
  return true
end

function song_mixer_status_text(lane)
  lane=clampi(lane or song_mixer_lane,1,3)
  ensure_song_mixer(song)
  local state=""
  if song_solo_lane==lane then state=state..", solo" end
  if song_lane_muted[lane] then state=state..", mute" end
  return "Operator "..lane.." Song Volume, "..clampi(song.mixer_volumes[lane] or 100,0,100).." percent"..state
end

function select_song_mixer_lane(lane)
  lane=clampi(lane,1,3)
  local preferred_param,preferred_cell=song_cursor_preference()
  local old_lane=song_lane
  local old_step=song_step
  local old_len=(old_lane>=1 and old_lane<=3) and song_lane_length(old_lane) or nil
  local old_speed_index=(old_lane>=1 and old_lane<=3 and is_song_speed_row()) and song_speed_index() or nil
  local section
  if old_lane==4 then section="steps"
  elseif old_step<0 then section="length"
  elseif old_step<old_len then section="steps"
  elseif old_step==old_len then section="repeat"
  else section="speed" end

  song_mixer_lane=lane
  if page==3 then
    clear_song_step_selection()
    song_lane=lane
    song_last_operator_lane=lane
    ensure_song_lane_meta(song)
    if section=="steps" then
      local remembered=(old_lane==4) and (song_info_step_by_lane[lane] or 0) or old_step
      song_step=clampi(remembered or 0,0,math.max(0,song_lane_length(lane)-1))
      song_info_step_by_lane[lane]=song_step
      restore_song_cursor_preference(preferred_param,preferred_cell)
    elseif section=="repeat" then
      song_step=song_lane_length(lane); song_cell=1
    elseif section=="speed" then
      local idx=clampi(old_speed_index or 1,1,song_lane_repeat_count(lane))
      song_step=song_lane_length(lane)+math.floor((idx-1)/10)+1
      song_cell=((idx-1)%10)+1
    else
      song_step=-1; song_cell=clampi(preferred_cell or 1,1,song_lane_meta_cell_count())
    end
    update_controller_title()
  end
  speak(song_mixer_status_text(song_mixer_lane))
end

function update_song_lane_monitoring()
  publish_song_mixer_state()
end

function toggle_song_lane_mute()
  local lane=clampi(song_mixer_lane or song_lane,1,3)
  song_lane_muted[lane]=not song_lane_muted[lane]
  if song_lane_muted[lane] and song_solo_lane==lane then song_solo_lane=0 end
  save_song_mixer_monitor_state(song)
  songs[current_song]=song
  biscot_record_song_undo_after_edit()
  write_active_song_mixer_headers()
  update_song_lane_monitoring()
  save_current_song_project_slot()
  speak("Operator "..lane..(song_lane_muted[lane] and ", mute" or ", mute off"))
end

function toggle_song_lane_solo()
  local lane=clampi(song_mixer_lane or song_lane,1,3)
  if song_solo_lane==lane then
    song_solo_lane=0
  else
    song_solo_lane=lane
    song_lane_muted[lane]=false
  end
  save_song_mixer_monitor_state(song)
  songs[current_song]=song
  biscot_record_song_undo_after_edit()
  write_active_song_mixer_headers()
  update_song_lane_monitoring()
  save_current_song_project_slot()
  speak(song_solo_lane==0 and "Solo off" or ("Operator "..lane..", solo"))
end

function change_song_mixer_volume(delta)
  local lane=clampi(song_mixer_lane or 1,1,3)
  ensure_song_mixer(song)
  local step=1
  local old=clampi(song.mixer_volumes[lane] or 100,0,100)
  local nv=clampi(old+delta*step,0,100)
  if nv==old then speak(song_mixer_status_text(lane)); return end
  song.mixer_volumes[lane]=nv
  songs[current_song]=song
  biscot_record_song_undo_after_edit()
  write_active_song_mixer_headers()
  publish_song_mixer_state()
  save_current_song_project_slot()
  speak("Operator "..lane.." Song Volume, "..nv.." percent")
end

function copy_current_song_parameter()
  if song_step<0 or song_step>=song_lane_length(song_lane) then return end
  local t=song.lanes[song_lane][song_step]
  local idx=song_current_param_index()
  if not t or not idx or (SONG_PARAM_LIST[idx] and SONG_PARAM_LIST[idx].info) then return end
  song_cell_clipboard={idx=idx,value=t[SONG_KEYS[idx]]}
  song_clipboard_mode="parameter"
  song_steps_enter_pending=false
  song_clipboard_enter_pending=true
  song_clipboard_song=current_song
  song_clipboard_lane=song_lane
  song_clipboard_step=song_step
  song_clipboard_param=idx
  speak("Copied "..SONG_PARAM_LIST[idx].name..", "..song_value_text(idx,t))
end

function cut_current_song_parameter()
  if song_step<0 or song_step>=song_lane_length(song_lane) then return end
  local t=song.lanes[song_lane][song_step]
  local idx=song_current_param_index()
  if not t or not idx or (SONG_PARAM_LIST[idx] and SONG_PARAM_LIST[idx].info) then return end
  local name=SONG_PARAM_LIST[idx].name
  local value_text_before=song_value_text(idx,t)
  copy_current_song_parameter()
  delete_song_parameter()
  speak("Cut "..name..", "..value_text_before)
end

function paste_current_song_parameter()
  if song_step<0 or song_step>=song_lane_length(song_lane) or not song_cell_clipboard then return end
  if song_clipboard_song==current_song and song_clipboard_lane==song_lane and song_clipboard_step==song_step then return end
  local idx=song_cell_clipboard.idx
  local t=ensure_song_step(song_lane,song_step)
  t._present=t._present or {}
  if t._present[idx] then return end
  t._present[idx]=true
  t[SONG_KEYS[idx]]=song_cell_clipboard.value
  t._order=t._order or {}; t._order[#t._order+1]=song_parameter_group_id(idx)
  local vis=song_visible_params(t)
  for i,id in ipairs(vis) do if id==idx then song_cell=i break end end
  refresh_song_after_edit()
  speak("Pasted "..SONG_PARAM_LIST[idx].name..", "..song_value_text(idx,t))
end

function paste_copied_song_parameter_to_next_steps()
  if not song_cell_clipboard then return end
  if song_clipboard_song~=current_song or song_clipboard_lane~=song_lane then return end
  if song_clipboard_step==nil or song_clipboard_step<0 then return end
  local last_step=clampi(song.length or 16,1,SONG_MAX_STEPS)-1
  if song_clipboard_step>=last_step then
    speak("No following song steps")
    return
  end
  local available=last_step-song_clipboard_step
  local ok,text=reaper.GetUserInputs("Copy Song parameter to following steps",1,"Number of following steps:",tostring(math.min(8,available)))
  restore_controller_focus_after_dialog()
  if not ok then return end
  local requested=tonumber(tostring(text or ""):match("^%s*(%-?%d+)%s*$"))
  if not requested or requested<1 then speak("Invalid number of steps"); return end
  requested=clampi(requested,1,available)
  local idx=song_cell_clipboard.idx
  local copied,skipped=0,0
  for st=song_clipboard_step+1,song_clipboard_step+requested do
    local t=ensure_song_step(song_lane,st)
    t._present=t._present or {}
    if t._present[idx] then
      skipped=skipped+1
    else
      t._present[idx]=true
      t[SONG_KEYS[idx]]=song_cell_clipboard.value
      copied=copied+1
    end
  end
  if copied>0 then refresh_song_after_edit() end
  if skipped>0 then
    speak(SONG_PARAM_LIST[idx].name.." copied to "..copied.." following steps. "..skipped.." skipped")
  else
    speak(SONG_PARAM_LIST[idx].name.." copied to "..copied.." following steps")
  end
end

function repeat_copied_song_steps()
  if not song_steps_clipboard or #song_steps_clipboard==0 then return false end
  if song_steps_clipboard_song~=current_song or song_steps_clipboard_lane~=song_lane then
    speak("Copied steps belong to another song or operator")
    return false
  end
  local source_end=clampi(song_steps_clipboard_source_end or song_step,0,math.max(0,song_lane_length(song_lane)-1))
  local block_size=#song_steps_clipboard
  local available=math.max(0,(song_lane_length(song_lane)-1)-source_end)
  local max_copies=math.floor(available/block_size)
  if max_copies<1 then
    speak("No space after selected steps")
    return false
  end
  local ok,text=reaper.GetUserInputs("Repeat copied Song steps",1,"Number of copies, 1 to "..tostring(max_copies)..":",tostring(math.min(2,max_copies)))
  restore_controller_focus_after_dialog()
  if not ok then return false end
  local requested=tonumber(tostring(text or ""):match("^%s*(%d+)%s*$"))
  if not requested or requested<1 or requested>max_copies then
    speak("Invalid number of copies")
    return false
  end
  local dst=source_end+1
  local written=0
  for copy_index=1,requested do
    for i,t in ipairs(song_steps_clipboard) do
      local target=dst+written
      if target>=song.length then break end
      if t and t._empty then
        song.lanes[song_lane][target]=nil
      else
        song.lanes[song_lane][target]=clone_song_step_data(t)
      end
      written=written+1
    end
  end
  if written<=0 then return false end
  refresh_song_after_edit()
  clear_song_step_selection()
  song_step=clampi(dst+written-1,0,song_lane_length(song_lane)-1)
  song_cell=1
  speak(tostring(requested)..(requested==1 and " copy inserted" or " copies inserted"))
  return true
end

function rename_current_song()
  local current=tostring(song_names[current_song] or song.name or "")
  local ok,input=reaper.GetUserInputs("Rename Song "..tostring(current_song),1,"Song name:",current)
  restore_controller_focus_after_dialog()
  if not ok then return false end
  local n=tostring(input or ""):gsub("[\r\n\t]"," "):gsub("^%s+",""):gsub("%s+$","")
  set_song_name(current_song,n)
  song.name=(n~="" and n or nil)
  songs[current_song]=song
  save_song_project()
  local active_path=(read_text_file(active_song_file_path()) or ""):gsub("[\r\n]","")
  if active_path~="" and file_exists(active_path) then write_text_file(active_path,serialize_song(song)) end
  update_controller_title()
  if n=="" then speak("Song "..tostring(current_song)..", name removed") else speak("Song "..tostring(current_song)..", "..n) end
  return true
end

function nearest_song_pattern_from_cursor()
  local lane=clampi((song_lane>=1 and song_lane<=3) and song_lane or song_mixer_lane or song_last_operator_lane or 1,1,3)
  local len=song_lane_length(lane)
  if len<=0 then return nil end
  local origin
  if song_lane==lane and song_step>=0 and song_step<len then origin=song_step
  else origin=clampi(song_info_step_by_lane[lane] or 0,0,len-1) end
  for distance=0,len-1 do
    local before=origin-distance
    if before>=0 then
      local t=song.lanes[lane][before]
      if t and (t.pattern or -1)>=0 then return lane,before,t end
    end
    local after=origin+distance
    if distance>0 and after<len then
      local t=song.lanes[lane][after]
      if t and (t.pattern or -1)>=0 then return lane,after,t end
    end
  end
  return nil
end

function jump_song_cursor_to_nearest_pattern()
  local origin_context={
    song=current_song,
    lane=song_lane,
    step=song_step,
    cell=song_cell,
    logical_param=song_logical_param,
    logical_cell=song_logical_cell,
    mixer_lane=song_mixer_lane
  }
  local lane,source_step,t=nearest_song_pattern_from_cursor()
  if not t then
    local requested_lane=clampi((song_lane>=1 and song_lane<=3) and song_lane or song_mixer_lane or 1,1,3)
    speak("No Pattern in Operator "..requested_lane)
    return false
  end
  local target=clampi(t.pattern or 0,0,MAX_PATTERNS-1)
  song_pattern_return_context=origin_context
  if song_playing then song_playing=false; write_song_memory(false) end
  activate_pattern(target)
  current_pattern=target
  local plen=clampi(ensure_pattern(target).length or 1,1,MAX_STEPS)
  current_step=clampi(t.pattern_start or 0,0,plen-1)
  last_real_step=current_step
  current_cell=1
  logical_cell=1
  write_text_file(active_pattern_path(),tostring(current_pattern))
  write_text_file(active_pattern_file_path(),"")
  if reaper.SetProjExtState then
    reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"active_pattern",tostring(current_pattern))
    reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"active_pattern_file","")
  end
  write_pattern_memory()
  save_ram_pattern_snapshot()
  biscot_change_page(1)
  speak("Pattern "..target.." from Operator "..lane..", Song step "..source_step..". Pattern step "..current_step)
  if is_real_step() then preview_pattern_step(current_step) end
  return true
end

function handle_f3(ch)
  if not ctrl() and not alt() and not shift() and (ch==string.byte("d") or ch==string.byte("D")) then toggle_core_model(); return end
  if not ctrl() and not alt() and not shift() and (ch==string.byte("o") or ch==string.byte("O")) then toggle_core_audio_mode(); return end
  if ctrl() and not alt() and not shift() and ch==K.CTRL_R then rename_current_song(); return end
  if ctrl() and not alt() and not shift() and ch==K.CTRL_J then jump_to_song_step(); return end
  if ctrl() and not alt() and not shift() and ch==K.CTRL_Z then biscot_undo_song(); return end
  if not ctrl() and not alt() and (ch==string.byte("1") or ch==string.byte("2") or ch==string.byte("3")) then select_song_mixer_lane(ch-string.byte("0")); return end
  if not ctrl() and not alt() and ch==string.byte("+") then change_song_mixer_volume(1); return end
  if not ctrl() and not alt() and ch==string.byte("-") then change_song_mixer_volume(-1); return end
  if (ch==string.byte("m") or ch==string.byte("M")) and not ctrl() and not alt() then toggle_song_lane_mute(); return end
  if (ch==string.byte("s") or ch==string.byte("S")) and not ctrl() and not alt() then toggle_song_lane_solo(); return end
  if song_lane<=3 and shift() and not ctrl() and not alt() and ch==K.UP then extend_song_step_selection(-1); return end
  if song_lane<=3 and shift() and not ctrl() and not alt() and ch==K.DOWN then extend_song_step_selection(1); return end
  if song_lane<=3 and shift() and not ctrl() and not alt() and ch==K.PAGE_UP then extend_song_step_selection(-5); return end
  if song_lane<=3 and shift() and not ctrl() and not alt() and ch==K.PAGE_DOWN then extend_song_step_selection(5); return end
  if ctrl() and shift() and alt() and (ch==K.CTRL_C or ch==string.byte("C") or ch==string.byte("c") or (gfx.getchar and (gfx.getchar(string.byte("C"))>0 or gfx.getchar(string.byte("c"))>0))) then
    copy_entire_current_song()
    return
  end
  if ctrl() and ch==K.CTRL_X then
    if song_step_selection_active() then cut_selected_song_steps() else cut_current_song_parameter() end
    return
  end
  if ctrl() and ch==K.CTRL_C then
    if song_step_selection_active() then copy_selected_song_steps() else copy_current_song_parameter() end
    return
  end
  if ctrl() and ch==K.CTRL_V then
    -- Ctrl+V follows the most recent copy operation. A stale full-Song TXT in
    -- the operating-system clipboard must not override a later step or
    -- parameter copy made inside the tracker.
    if song_clipboard_mode=="steps" then
      paste_selected_song_steps()
    elseif song_clipboard_mode=="parameter" then
      paste_current_song_parameter()
    elseif song_clipboard_mode=="full_song" then
      paste_entire_song()
    else
      -- Also accept a Biscot1 Song TXT copied by another controller session.
      local cliptxt=biscot_get_text_from_clipboard()
      if cliptxt and cliptxt:find("BISCOT1_TRACKER_SONG_V1",1,true) then
        whole_song_clipboard=cliptxt
        song_clipboard_mode="full_song"
        paste_entire_song()
      end
    end
    return
  end
  if ch==K.ENTER then
    local now=reaper.time_precise and reaper.time_precise() or 0
    if now<ignore_enter_until then return end
    if song_steps_enter_pending and song_clipboard_mode=="steps" then
      song_steps_enter_pending=false
      repeat_copied_song_steps()
    elseif song_clipboard_enter_pending and song_cell_clipboard and song_clipboard_song==current_song and song_clipboard_lane==song_lane then
      song_clipboard_enter_pending=false
      paste_copied_song_parameter_to_next_steps()
    else
      jump_to_song_number()
    end
    return
  end
  if ctrl() and not alt() and not shift() and ch==K.UP then change_current_song(-1); return end
  if ctrl() and not alt() and not shift() and ch==K.DOWN then change_current_song(1); return end
  if ctrl() and shift() and ch==K.CTRL_S then
    local ok,path=save_composition_with_name()
    if ok then speak("Composition saved, "..base_name(path)) else speak("Composition save cancelled") end
    return
  end
  if ctrl() and not shift() and ch==K.CTRL_S then
    local ok,path=save_song_txt_with_name()
    if ok then
      refresh_f5_entries(base_name(path))
      speak("Saved "..base_name(path):gsub("%.txt$","")..". Patterns and Song")
    elseif path~="" then
      speak("Song save failed")
    end
    return
  end
  if ctrl() and ch==K.TAB then
    clear_song_step_selection()
    jump_song_cursor_to_nearest_pattern()
    return
  end
  if ch==K.TAB then speak_song_position(); return end
  if ch==K.SPACE then
    -- gfx.getchar can repeat Space while the key is still held. Without a
    -- guard one physical press may execute Stop and immediately Play again.
    local now=(reaper.time_precise and reaper.time_precise()) or os.clock()
    if now-song_space_last_time < SONG_SPACE_DEBOUNCE then return end
    song_space_last_time=now
    if song_playing then
      song_playing=false
      song_play_start_time=-1000
      write_song_memory(false)
      speak("Song stopped")
    else
      biscot_prepare_preview("song")
      song_playing=true
      song_play_start_time=now
      write_song_memory(true)
    end
    return
  end
  -- Value editing mirrors F1. Modifier handlers must precede plain navigation.
  if ctrl() and not alt() and not shift() and ch==K.LEFT then reorder_current_song_parameter(-1); return end
  if ctrl() and not alt() and not shift() and ch==K.RIGHT then reorder_current_song_parameter(1); return end
  if alt() and ch==K.LEFT then set_jump_index(jump_index-1); return end
  if alt() and ch==K.RIGHT then set_jump_index(jump_index+1); return end
  if alt() and ch==K.UP then change_song_value(1,false); return end
  if alt() and ch==K.DOWN then change_song_value(-1,false); return end
  if alt() and ch==K.PAGE_UP then change_song_value(1,true); return end
  if alt() and ch==K.PAGE_DOWN then change_song_value(-1,true); return end
  if alt() and ch==K.HOME then set_song_value_extreme(false); return end
  if alt() and ch==K.ENDK then set_song_value_extreme(true); return end
  local pg=5
  if ch==K.HOME then
    -- Home scende tra i quattro campi principali:
    -- Length -> Steps -> Repeat/Speed -> Global.
    if song_lane==4 then return end
    if song_step<0 then
      song_step=0; song_cell=1; speak_song_position(true)
    elseif song_step>=0 and song_step<song_lane_length(song_lane) then
      song_step=song_lane_length(song_lane); song_cell=1; speak_song_position(true)
    elseif song_step==song_lane_length(song_lane) or is_song_speed_row() then
      song_last_operator_lane=song_lane; song_lane=4; song_step=-1; song_cell=1; speak_song_position(true)
    end
    return
  end
  if ch==K.ENDK then
    -- End sale in modo perfettamente speculare:
    -- Global -> Repeat/Speed -> Steps -> Length.
    if song_lane==4 then
      song_lane=clampi(song_last_operator_lane or 1,1,3)
      song_step=song_lane_length(song_lane)
      song_cell=1; speak_song_position(true); return
    end
    if is_song_speed_row() then
      song_step=song_lane_length(song_lane); song_cell=1; speak_song_position(true)
    elseif song_step==song_lane_length(song_lane) then
      song_step=math.max(0,song_lane_length(song_lane)-1); song_cell=1; speak_song_position(true)
    elseif song_step>=0 and song_step<song_lane_length(song_lane) then
      song_step=-1; song_cell=1; speak_song_position(true)
    end
    return
  end
  if ch==K.UP then
    if song_lane==4 then if song_cell>1 then song_cell=song_cell-1; speak_song_position(true) end; return end
    if song_plain_arrow_is_shift_echo(-1) then return end
    clear_song_step_selection()
    move_song_vertical_bounded(-1)
    return
  end
  if ch==K.DOWN then
    if song_lane==4 then local maxc=song_global_cell_count(); if song_cell<maxc then song_cell=song_cell+1; speak_song_position(true) end; return end
    if song_plain_arrow_is_shift_echo(1) then return end
    clear_song_step_selection()
    move_song_vertical_bounded(1)
    return
  end
  if ch==K.PAGE_UP then
    if song_plain_arrow_is_shift_echo(-pg) then return end
    clear_song_step_selection(); move_song_vertical_bounded(-pg); return
  end
  if ch==K.PAGE_DOWN then
    if song_plain_arrow_is_shift_echo(pg) then return end
    clear_song_step_selection(); move_song_vertical_bounded(pg); return
  end
  if ch==K.LEFT then
    local maxc
    if song_lane==4 then maxc=song_global_cell_count() elseif song_is_lane_meta_row() then maxc=song_lane_meta_cell_count() elseif song_step==song_lane_length(song_lane) then maxc=1 elseif is_song_speed_row() then maxc=song_speed_cell_count() else maxc=#song_visible_params(song.lanes[song_lane][song_step]) end
    if maxc>0 then local old=song_cell; song_cell=clampi(song_cell-1,1,maxc); if song_cell~=old then speak_song_position(true) end end
    return
  end
  if ch==K.RIGHT then
    local maxc
    if song_lane==4 then maxc=song_global_cell_count() elseif song_is_lane_meta_row() then maxc=song_lane_meta_cell_count() elseif song_step==song_lane_length(song_lane) then maxc=1 elseif is_song_speed_row() then maxc=song_speed_cell_count() else maxc=#song_visible_params(song.lanes[song_lane][song_step]) end
    if maxc>0 then local old=song_cell; song_cell=clampi(song_cell+1,1,maxc); if song_cell~=old then speak_song_position(true) end end
    return
  end
  if ch==K.DELETE then
    if ctrl() and alt() and shift() then reset_current_song(); return end
    if ctrl() and shift() and not alt() then clear_song_lane(); return end
    if ctrl() and not shift() and not alt() then clear_song_step(); return end
    if not ctrl() and not shift() and not alt() then
      if song_step_selection_active() then delete_selected_song_steps() else delete_song_parameter() end
      return
    end
  end
end
function f4_row(pos) return ((pos-1)%param_grid_height)+1 end
function f4_col(pos) return math.floor((pos-1)/param_grid_height)+1 end
function f4_speak()
  local t=(song_step>=0 and song_step<song_lane_length(song_lane)) and song.lanes[song_lane][song_step] or nil
  local param_id=F4_PARAM_IDS[f4_index]
  if t and param_id and song_param_present(t,param_id) then
    speak(F4_LIST[f4_index]..", "..song_value_text(param_id,t))
  else
    speak(F4_LIST[f4_index])
  end
end
function f4_move(target)
  target=clampi(target,1,#F4_LIST)
  if target==f4_index then return end
  f4_index=target
  f4_speak()
end
function add_f4_song_parameter(stay_in_f4)
  if song_step<0 or song_step>=song_lane_length(song_lane) then speak("Select a Song step"); return false end
  local t=ensure_song_step(song_lane,song_step); t._present=t._present or {}
  local name=F4_LIST[f4_index] or "Parameter"
  local param_id=F4_PARAM_IDS[f4_index]
  local was_present=t._present[param_id] and true or false
  t._order=t._order or {}
  if param_id==1 then
    -- Pattern is one F4 choice but expands to the complete Pattern group in F3.
    t._present[1]=true
    t._present[5]=true
    t._present[6]=true
    local found=false; for _,id in ipairs(t._order) do if id==1 then found=true; break end end
    if not found then t._order[#t._order+1]=1 end
    if t.pattern_start==nil then t.pattern_start=0 end
    local plen=clampi(((t.pattern or -1)>=0 and ensure_pattern(t.pattern).length) or 1,1,MAX_STEPS)
    if not tonumber(t.play_length) or tonumber(t.play_length)<=0 then t.play_length=math.max(1,plen-clampi(t.pattern_start or 0,0,plen-1)) end
    refresh_song_after_edit()
  elseif not was_present then
    t._present[param_id]=true; t[SONG_KEYS[param_id]]=SONG_DEFAULTS[param_id]
    t._order[#t._order+1]=param_id
    refresh_song_after_edit()
  end
  local vis=song_visible_params(t)
  for i,id in ipairs(vis) do if id==param_id then song_cell=i break end end
  if stay_in_f4 then
    if was_present then f4_speak() else speak(name.." added") end
  else
    biscot_change_page(3); speak_song_position()
  end
  return true
end
function handle_f4(ch)
  if song_step<0 or song_step>=song_lane_length(song_lane) then biscot_change_page(3); speak_song_position(); return end
  if parameter_jump_to_initial(ch,F4_LIST,f4_index,f4_move) then return end

  if ctrl() and ch==K.LEFT then
    if param_grid_height<=1 then return end
    param_grid_height=param_grid_height-1
    f4_index=clampi(f4_index,1,#F4_LIST)
    speak("Grid "..param_grid_height)
    return
  end
  if ctrl() and ch==K.RIGHT then
    if param_grid_height>=100 then return end
    param_grid_height=param_grid_height+1
    f4_index=clampi(f4_index,1,#F4_LIST)
    speak("Grid "..param_grid_height)
    return
  end

  local row=f4_row(f4_index)
  local col=f4_col(f4_index)
  local col_first=(col-1)*param_grid_height+1
  local col_last=math.min(#F4_LIST,col*param_grid_height)

  if ch==K.UP then
    if row<=1 then return end
    f4_move(f4_index-1); return
  end
  if ch==K.DOWN then
    if f4_index>=col_last then return end
    f4_move(f4_index+1); return
  end
  if ch==K.LEFT then
    if col<=1 then return end
    local target=f4_index-param_grid_height
    if target<1 then return end
    f4_move(target); return
  end
  if ch==K.RIGHT then
    local target=f4_index+param_grid_height
    if target>#F4_LIST then return end
    f4_move(target); return
  end

  -- Identical to F2: Ctrl+Home/End address the whole grid.
  if ctrl() and ch==K.HOME then f4_move(#F4_LIST); return end
  if ctrl() and ch==K.ENDK then f4_move(1); return end

  -- Identical to F2: Home/End address the current column.
  if ch==K.HOME then f4_move(col_last); return end
  if ch==K.ENDK then f4_move(col_first); return end

  local pg=math.max(5,jump_sizes[jump_index])
  if ch==K.PAGE_UP then f4_move(f4_index-pg); return end
  if ch==K.PAGE_DOWN then f4_move(f4_index+pg); return end

  -- In some REAPER/gfx builds Ctrl+Enter is reported as LF (10) rather than Enter (13).
  -- Accept both forms so F4 matches the reliable Ctrl+Enter behavior expected from F2.
  if ctrl() and (ch==K.ENTER or ch==10) then add_f4_song_parameter(true); return end
  if ch==K.ENTER then add_f4_song_parameter(false); return end
end


local f6_diag_lines={}
local f6_diag_index=1
local f6_paused=false -- retained only for compatibility with the common Stop path

local function diag_read(offset)
  return tonumber(reaper.gmem_read(DIAG_BASE+offset) or 0) or 0
end
local function diag_op_read(op,offset)
  return tonumber(reaper.gmem_read(DIAG_OP_BASE+(op-1)*DIAG_OP_STRIDE+offset) or 0) or 0
end
local function diag_i(v)
  return math.floor((tonumber(v) or 0)+0.5)
end
local function diag_event_name(v)
  local n=diag_i(v)
  return ({[0]="None",[1]="Note On",[2]="Note Off",[3]="All Off",[4]="Parameter",[5]="Legato",[6]="Sidechain",[7]="Delay",[8]="Delay Volume",[9]="Delay Ping Pong",[10]="Operator Reset",[11]="Gate Off"})[n] or ("Event "..n)
end
local function diag_stage_name(v)
  return ({[0]="Idle",[1]="Attack",[2]="Decay",[3]="Sustain",[4]="Release"})[diag_i(v)] or tostring(diag_i(v))
end
local function diag_pending(write_pos,read_pos)
  write_pos=diag_i(write_pos)%QUEUE_CAPACITY
  read_pos=diag_i(read_pos)%QUEUE_CAPACITY
  return write_pos>=read_pos and (write_pos-read_pos) or (QUEUE_CAPACITY-(read_pos-write_pos))
end
local function diag_source_text(op)
  local pat=diag_i(diag_op_read(op,12))
  local st=diag_i(diag_op_read(op,13))
  local sn=diag_i(diag_op_read(op,14))
  local row=diag_i(diag_op_read(op,15))
  local pr=diag_i(diag_op_read(op,16))+1
  local rr=diag_i(diag_op_read(op,17))+1
  local last_note=diag_i(diag_op_read(op,46))
  local note_text=(last_note>=0 and last_note<=127) and note_name(last_note) or "unknown note"
  if pat<0 then return "source unavailable, "..note_text end
  if sn>=0 then
    return string.format("Song %d row %d, Pattern %d step %d, Pattern repeat %d, row repeat %d, %s",sn,row,pat,st,pr,rr,note_text)
  end
  return string.format("Pattern %d step %d, repeat %d, %s",pat,st,pr,note_text)
end

function build_f6_diagnostic_report()
  reaper.gmem_attach(GMEM_NS)
  local lines={}
  local magic=diag_i(diag_read(0))
  local version=diag_i(diag_read(1))
  local serial=diag_i(diag_read(2))
  local protocol=diag_i(reaper.gmem_read(0) or 0)
  local play=diag_i(reaper.gmem_read(PLAY_STATUS) or 0)
  if magic~=DIAG_MAGIC then
    lines[#lines+1]="Diagnostics: no session recorded. Start the Song, reproduce the problem, then return to F6 and press Control C."
    lines[#lines+1]="Protocol seen: "..protocol..". Expected: 20261013."
    return table.concat(lines,"\r\n"),lines
  end
  lines[#lines+1]=string.format("Biscot1 F6 diagnostics version %d, session %d.",version,serial)
  lines[#lines+1]=string.format("Protocol %d. Play status %d. Launch Song %d. Start clock %d. Tracker clock %d. Sample rate %d. Block %d.",protocol,play,diag_i(diag_read(4)),diag_i(diag_read(3)),diag_i(diag_read(12)),diag_i(diag_read(14)),diag_i(diag_read(15)))
  lines[#lines+1]=string.format("Song starts %d. Internal Song repeats %d. Song transitions %d. Last boundary target %d.",diag_i(diag_read(6)),diag_i(diag_read(7)),diag_i(diag_read(8)),diag_i(diag_read(9)))
  lines[#lines+1]=string.format("Chronology repairs: Op1 %d, Op2 %d, Op3 %d. Maximum backward distance: Op1 %d, Op2 %d, Op3 %d samples. Last backward distance: Op1 %d, Op2 %d, Op3 %d samples.",diag_i(diag_read(16)),diag_i(diag_read(17)),diag_i(diag_read(18)),diag_i(diag_read(20)),diag_i(diag_read(21)),diag_i(diag_read(22)),diag_i(diag_read(24)),diag_i(diag_read(25)),diag_i(diag_read(26)))
  for op=1,3 do
    local tq_on=diag_i(diag_op_read(op,0)); local tq_off=diag_i(diag_op_read(op,1)); local tq_leg=diag_i(diag_op_read(op,2))
    local tq_gate=diag_i(diag_op_read(op,3)); local tq_all=diag_i(diag_op_read(op,4)); local tq_reset=diag_i(diag_op_read(op,5))
    local drops=diag_i(diag_op_read(op,6)); local sched_late=diag_i(diag_op_read(op,7))
    local cr_on=diag_i(diag_op_read(op,20)); local cr_off=diag_i(diag_op_read(op,21)); local cr_leg=diag_i(diag_op_read(op,22))
    local cr_gate=diag_i(diag_op_read(op,23)); local cr_all=diag_i(diag_op_read(op,24)); local cr_reset=diag_i(diag_op_read(op,25))
    local attacks=diag_i(diag_op_read(op,26)); local noatt=diag_i(diag_op_read(op,27)); local late=diag_i(diag_op_read(op,28)); local order=diag_i(diag_op_read(op,29))
    local readp=diag_i(reaper.gmem_read(7+op) or diag_op_read(op,39))
    local writep=diag_i(reaper.gmem_read(3+op) or diag_op_read(op,40))
    local pending=diag_pending(writep,readp)
    local core_last_note=diag_i(diag_op_read(op,47))
    local core_note_text=(core_last_note>=0 and core_last_note<=127) and note_name(core_last_note) or "none"
    lines[#lines+1]=string.format("Operator %d summary: Note On queued %d, received %d, difference %d, Attack %d, no Attack %d, drops %d, scheduled late %d, received late %d, order errors %d, queue pending %d, last Core Note On %s.",op,tq_on,cr_on,tq_on-cr_on,attacks,noatt,drops,sched_late,late,order,pending,core_note_text)
    lines[#lines+1]=string.format("Operator %d Tracker: Note Off %d, Legato %d, Gate Off %d, All Off %d, Operator Reset %d. Last %s, value %.3f, target %d, LFO mask %.0f; %s.",op,tq_off,tq_leg,tq_gate,tq_all,tq_reset,diag_event_name(diag_op_read(op,8)),diag_op_read(op,9),diag_i(diag_op_read(op,10)),diag_op_read(op,44),diag_source_text(op))
    lines[#lines+1]=string.format("Operator %d Core: Note Off %d, Legato %d, Gate Off %d, All Off %d, Operator Reset %d. Last %s, value %.3f, target %d, last lateness %d samples.",op,cr_off,cr_leg,cr_gate,cr_all,cr_reset,diag_event_name(diag_op_read(op,30)),diag_op_read(op,31),diag_i(diag_op_read(op,32)),diag_i(diag_op_read(op,33)))
    lines[#lines+1]=string.format("Operator %d state: note %d, envelope %s %.6f, gate %.3f, held notes %d, filter-held %d, read %d, write %d.",op,diag_i(diag_op_read(op,34)),diag_stage_name(diag_op_read(op,35)),diag_op_read(op,36),diag_op_read(op,37),diag_i(diag_op_read(op,38)),diag_i(diag_op_read(op,43)),readp,writep)
    if drops>0 then
      lines[#lines+1]=string.format("Operator %d last dropped event: %s at target %d.",op,diag_event_name(diag_op_read(op,18)),diag_i(diag_op_read(op,19)))
    end
  end
  return table.concat(lines,"\r\n"),lines
end

function f6_reset_diagnostics()
  reaper.gmem_attach(GMEM_NS)
  local serial=diag_i(diag_read(2))+1
  for i=0,DIAG_SIZE-1 do reaper.gmem_write(DIAG_BASE+i,0) end
  reaper.gmem_write(DIAG_BASE+0,DIAG_MAGIC)
  reaper.gmem_write(DIAG_BASE+1,DIAG_VERSION)
  reaper.gmem_write(DIAG_BASE+2,serial)
  reaper.gmem_write(DIAG_BASE+3,reaper.gmem_read(1) or 0)
  reaper.gmem_write(DIAG_BASE+4,current_song)
  reaper.gmem_write(DIAG_BASE+5,2)
  reaper.gmem_write(DIAG_BASE+11,9)
  for op=1,3 do
    local b=DIAG_OP_BASE+(op-1)*DIAG_OP_STRIDE
    for _,off in ipairs({10,11,12,13,14,15,19,32,34,41,46,47}) do reaper.gmem_write(b+off,-1) end
  end
  f6_diag_index=1
  f6_speak()
end

function f6_speak()
  local _,lines=build_f6_diagnostic_report()
  f6_diag_lines=lines
  f6_diag_index=clampi(f6_diag_index,1,math.max(1,#lines))
  speak(lines[f6_diag_index] or "Diagnostics empty")
end
function f6_move(delta)
  local _,lines=build_f6_diagnostic_report()
  f6_diag_lines=lines
  local ni=clampi(f6_diag_index+delta,1,math.max(1,#lines))
  f6_diag_index=ni
  speak(lines[ni] or "Diagnostics empty")
end
function handle_f6(ch)
  if ctrl() and not shift() and not alt() and (ch==K.CTRL_C or ch==string.byte("c") or ch==string.byte("C")) then
    local report=build_f6_diagnostic_report()
    if biscot_copy_text_to_clipboard(report) then speak("Complete diagnostic report copied to clipboard") else speak("Clipboard unavailable") end
    return
  end
  if not ctrl() and not alt() and (ch==string.byte("r") or ch==string.byte("R")) then f6_reset_diagnostics(); return end
  if ch==K.UP then f6_move(-1); return end
  if ch==K.DOWN then f6_move(1); return end
  if ch==K.PAGE_UP then f6_move(-5); return end
  if ch==K.PAGE_DOWN then f6_move(5); return end
  if ch==K.HOME then f6_diag_index=1; f6_speak(); return end
  if ch==K.ENDK then
    local _,lines=build_f6_diagnostic_report(); f6_diag_index=math.max(1,#lines); f6_speak(); return
  end
  if ch==K.SPACE or ch==K.ENTER then f6_speak(); return end
end

function f5_txt_kind(path)
  local txt=read_text_file(path)
  if not txt then return "unreadable" end
  local head=txt:sub(1,4096)
  if head:find("BISCOT1_TRACKER_PATTERN_TXT_V1",1,true) or
     (head:match("^pattern=%-?%d+") and head:find("\nlength=",1,true)) or
     (head:find("\nstep\t",1,true) and head:find("\ncell\t",1,true)) then
    return "pattern"
  end
  local name=base_name(path):lower()
  if head:find("BISCOT1_TRACKER_COMPOSITION_V8_RAW_GMEM",1,true) or head:find("BISCOT1_TRACKER_COMPOSITION_V7",1,true) or head:find("BISCOT1_TRACKER_COMPOSITION_V6",1,true) or head:find("BISCOT1_TRACKER_COMPOSITION_V5",1,true) or head:find("BISCOT1_TRACKER_COMPOSITION_V4",1,true) then return "composition" end
  if head:find("BISCOT1_TRACKER_SONG",1,true) or
     name:match("^song[%s_%-].*%.txt$") or name:match(".*[%s_%-]song%.txt$") then
    return "song"
  end
  return "other"
end


-- F5 uses an independent, non-destructive preview buffer.  It never writes
-- the selected TXT into the current Lua pattern slot.
f5_preview_active=false
f5_preview_path=""

function biscot_read_pattern_for_preview(path)
  local f=io.open(path,"r")
  if not f then return nil end
  local pat={length=16,repeat_count=1,repeat_speeds={1},repeat_transposes={0},midi_note=-1,release_arp=0,steps={}}
  for st=0,MAX_STEPS-1 do pat.steps[st]={{},{},{}} end
  local curst,curtab=nil,nil
  local valid=false
  for line in f:lines() do
    if line=="BISCOT1_TRACKER_PATTERN_TXT_V1" then valid=true
    elseif line:match("^length=") then pat.length=clampi(tonumber(line:match("^length=(%-?%d+)")) or 16,1,MAX_STEPS)
    elseif line:match("^repeat=") then pat.repeat_count=clampi(tonumber(line:match("^repeat=(%-?%d+)")) or 1,1,1000)
    elseif line:match("^release_arp=") then pat.release_arp=clampi(tonumber(line:match("^release_arp=(%-?%d+)")) or 0,0,1)
    elseif line:match("^speed\t") then
      local i,v=line:match("^speed\t([^\t]+)\t([^\t]+)"); i=tonumber(i); v=tonumber(v)
      if i then pat.repeat_speeds[i]=clamp(v or 1,0.25,16) end
    elseif line:match("^transpose\t") then
      local i,v=line:match("^transpose\t([^\t]+)\t([^\t]+)"); i=tonumber(i); v=tonumber(v)
      if i then pat.repeat_transposes[i]=clampi(v or 0,-96,96) end
    elseif line:match("^step\t") then
      local a,b=line:match("^step\t([^\t]+)\t([^\t]+)")
      curst=clampi(tonumber(a) or 0,0,MAX_STEPS-1); curtab=clampi(tonumber(b) or 1,1,3); valid=true
    elseif line:match("^cell\t") and curst~=nil and curtab then
      local a,b=line:match("^cell\t([^\t]+)\t([^\t]+)")
      if a then table.insert(pat.steps[curst][curtab],{id=tonumber(a) or 0,value=tonumber(b) or 0}) end
    end
  end
  f:close()
  if not valid then return nil end
  for i=1,pat.repeat_count do
    if pat.repeat_speeds[i]==nil then pat.repeat_speeds[i]=1 end
    if pat.repeat_transposes[i]==nil then pat.repeat_transposes[i]=0 end
  end
  return pat
end

function biscot_write_browser_preview(pat,run)
  reaper.gmem_attach(GMEM_NS)
  for st=0,MAX_STEPS-1 do
    local flat={}
    for tab=1,3 do
      for _,c in ipairs(pat.steps[st][tab] or {}) do
        if #flat<MAX_CELLS then flat[#flat+1]={id=resolved_id(c),value=tonumber(c.value) or 0} end
      end
    end
    local sb=step_base(st); reaper.gmem_write(sb,#flat)
    for i=1,MAX_CELLS do
      local cb=sb+1+(i-1)*CELL_SIZE; local c=flat[i]
      reaper.gmem_write(cb,c and c.id or 0); reaper.gmem_write(cb+1,c and c.value or 0)
    end
  end
  reaper.gmem_write(LUA_BASE+0,LUA_MAGIC)
  reaper.gmem_write(LUA_BASE+2,1)
  reaper.gmem_write(LUA_BASE+4,clampi(pat.length or 16,1,MAX_STEPS))
  local rc=clampi(pat.repeat_count or 1,1,1000)
  reaper.gmem_write(LUA_BASE+5,rc)
  reaper.gmem_write(LUA_BASE+6,1.0)
  reaper.gmem_write(LUA_BASE+7,1)
  for i=1,1000 do
    reaper.gmem_write(LUA_BASE+7+i,i<=rc and clamp(tonumber(pat.repeat_speeds[i]) or 1,0.25,16) or 0)
    reaper.gmem_write(REPEAT_TRANSPOSE_BASE+i-1,i<=rc and clampi(pat.repeat_transposes[i] or 0,-96,96) or 0)
  end
  lua_serial=lua_serial+1
  reaper.gmem_write(PLAY_STATUS,run and 1 or 0)
  reaper.gmem_write(LUA_BASE+3,run and 1 or 0)
  reaper.gmem_write(LUA_BASE+1,lua_serial)
end

function biscot_stop_f5_preview()
  if not f5_preview_active then return end
  reaper.gmem_write(PLAY_STATUS,0)
  reaper.gmem_write(LUA_BASE+3,0)
  f5_preview_active=false
  f5_preview_path=""
  playing=false
  write_pattern_memory() -- restore F1 RAM pattern without changing it
end

function biscot_start_f5_preview(path)
  local pat=biscot_read_pattern_for_preview(path)
  if not pat then return false end
  biscot_prepare_preview("browser")
  if f5_preview_active then biscot_stop_f5_preview() end
  f5_preview_active=true
  f5_preview_path=path
  playing=true
  biscot_write_browser_preview(pat,true)
  return true
end


function f5_jump_to_initial(ch)
  if ctrl() or shift() or alt() then return false end
  if ch<48 or (ch>57 and ch<65) or (ch>90 and ch<97) or ch>122 then return false end
  local wanted=string.char(ch):lower()
  local count=#f5_entries
  if count<1 then return true end
  for offset=1,count do
    local idx=((f5_index-1+offset)%count)+1
    local name=tostring(f5_entries[idx].name or ""):lower()
    if name:sub(1,1)==wanted then
      f5_index=idx
      f5_speak()
      return true
    end
  end
  return true
end

function handle_f5(ch)
  if ch==K.CTRL_D then
    if f5_preview_active then biscot_stop_f5_preview() end
    if choose_library_root() then
      f5_path=""; f5_scope_root=""; f5_index=1
      refresh_f5_entries("Library")
      speak("Library folder updated. Biscot1 Tracker")
    else
      speak("Library folder unchanged")
    end
    return
  end
  if ch==K.UP then if f5_index>1 then f5_index=f5_index-1; f5_speak() end; return end
  if ch==K.DOWN then if f5_index<#f5_entries then f5_index=f5_index+1; f5_speak() end; return end
  if ch==K.HOME then if #f5_entries>0 and f5_index~=#f5_entries then f5_index=#f5_entries; f5_speak() end; return end
  if ch==K.ENDK then if #f5_entries>0 and f5_index~=1 then f5_index=1; f5_speak() end; return end
  if ch==K.LEFT or ch==K.BACKSPACE or ch==K.BACKSPACE2 then f5_go_parent(); return end
  if ch==K.RIGHT then local e=f5_entries[f5_index]; if e and e.kind=="folder" then f5_enter_folder(e) end; return end
  if ch==K.F2 then rename_f5_current(); return end
  if ch==K.SPACE then
    local e=f5_entries[f5_index]
    if not e or e.kind~="file" or f5_txt_kind(e.path)~="pattern" then return end
    if f5_preview_active and f5_preview_path==e.path then
      biscot_stop_f5_preview()
    else
      biscot_start_f5_preview(e.path)
    end
    return
  end
  if ch==K.ENTER then
    local e=f5_entries[f5_index]; if not e then return end
    if e.kind=="folder" then f5_enter_folder(e); return end
    local kind=f5_txt_kind(e.path)
    if kind=="composition" then
      if f5_preview_active then biscot_stop_f5_preview() end
      if load_composition_txt(e.path) then
        biscot_change_page(3)
        speak("Composition loaded. Song "..current_song..". Patterns and Songs")
      else
        speak("Composition load failed")
      end
      return
    end
    if kind=="song" then
      if f5_preview_active then biscot_stop_f5_preview() end
      if song_has_content(song) then
        biscot_begin_overwrite_prompt(e.path,current_song,"song")
      else
        biscot_begin_overwrite_prompt(e.path,current_song,"song")
        biscot_finish_overwrite_prompt(true)
      end
      return
    end
    if kind~="pattern" then return end
    if f5_preview_active then biscot_stop_f5_preview() end
    if pattern_has_content(patterns[current_pattern]) then
      biscot_begin_overwrite_prompt(e.path,current_pattern,"pattern")
    else
      biscot_begin_overwrite_prompt(e.path,current_pattern,"pattern")
      biscot_finish_overwrite_prompt(true)
    end
    return
  end
  if f5_jump_to_initial(ch) then return end
  -- Ctrl+S is intentionally inactive in F5. Saving is contextual: F1 saves Pattern, F3 will save Song.
end

-- In-controller confirmation prompt. It keeps the gfx window focused and uses
-- fixed English Yes/No labels instead of REAPER's localized MessageBox.
overwrite_prompt_active=false
overwrite_prompt_choice=2 -- 1 Yes, 2 No
overwrite_prompt_path=""
overwrite_prompt_destination=0
overwrite_prompt_kind="pattern"
overwrite_prompt_mouse_was_down=false

function biscot_begin_overwrite_prompt(path,destination,kind)
  overwrite_prompt_active=true
  overwrite_prompt_choice=2
  overwrite_prompt_path=path or ""
  overwrite_prompt_destination=destination or current_pattern
  overwrite_prompt_kind=kind or "pattern"
  overwrite_prompt_mouse_was_down=false
  local label=(overwrite_prompt_kind=="song") and "Song" or "Pattern"
  speak("Overwrite "..label.." "..tostring(overwrite_prompt_destination).."? No")
end

function biscot_draw_overwrite_prompt()
  gfx.set(0.12,0.12,0.12,1); gfx.rect(0,0,gfx.w,gfx.h,1)
  gfx.set(1,1,1,1); gfx.x=18; gfx.y=12
  local label=(overwrite_prompt_kind=="song") and "Song" or "Pattern"
  gfx.drawstr("Overwrite "..label.." "..tostring(overwrite_prompt_destination).." with selected TXT?")
  if overwrite_prompt_choice==1 then gfx.set(0.55,0.55,0.55,1) else gfx.set(0.25,0.25,0.25,1) end
  gfx.rect(90,48,150,36,1)
  if overwrite_prompt_choice==2 then gfx.set(0.55,0.55,0.55,1) else gfx.set(0.25,0.25,0.25,1) end
  gfx.rect(500,48,150,36,1)
  gfx.set(1,1,1,1); gfx.x=148; gfx.y=58; gfx.drawstr("Yes")
  gfx.x=560; gfx.y=58; gfx.drawstr("No")
end

function biscot_finish_overwrite_prompt(accepted)
  local path=overwrite_prompt_path
  local destination=overwrite_prompt_destination
  local kind=overwrite_prompt_kind
  overwrite_prompt_active=false
  overwrite_prompt_path=""
  if not accepted then
    -- Return to the browser item that opened the prompt.
    local selected_name=base_name(path)
    refresh_f5_entries(selected_name)
    f5_speak()
    return
  end
  if kind=="pattern" then
    local ok=load_pattern_txt(path,destination)
    if ok then
      current_pattern=destination; ram_pattern_loaded[destination]=true
      set_pattern_name(destination,path)
      current_step=-1; current_cell=1; logical_cell=1
      local _,local_path=save_pattern_txt(destination)
      write_text_file(active_pattern_path(),tostring(destination))
      write_text_file(active_pattern_file_path(),norm_path(local_path or pattern_path(destination)))
      if reaper.SetProjExtState then reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"active_pattern_file",norm_path(local_path or pattern_path(destination))) end
      -- The Song engine does not read the single F1 preview buffer: it reads
      -- the complete GMEM pattern bank. Rebuild it immediately after import.
      save_ram_pattern_snapshot(destination,true)
      rebuild_midi_pattern_bank()
      write_pattern_memory(); playing=false
      -- Keep the imported pattern available after closing and reopening the
      -- controller, even in an unsaved/default session.
      save_session_state()
      page=1
      speak("Pattern "..tostring(destination).." loaded. F1")
    end
  elseif kind=="song" then
    if load_song_txt(path) then
      for p=0,MAX_PATTERNS-1 do
        if patterns[p] and pattern_has_content(patterns[p]) then save_ram_pattern_snapshot(p,true) end
      end
      rebuild_midi_pattern_bank()
      write_song_memory(false)
      save_session_state()
      song_step=-1; song_cell=1; song_lane=1
      biscot_change_page(3)
      if restore_controller_focus_after_dialog then restore_controller_focus_after_dialog() end
      speak("Song "..current_song.." loaded. Patterns and Song. F3")
    else
      speak("Song load failed")
    end
  end
end

function biscot_handle_overwrite_prompt(ch)
  if ch==K.ESC or ch==string.byte("n") or ch==string.byte("N") then biscot_finish_overwrite_prompt(false); return true end
  if ch==string.byte("y") or ch==string.byte("Y") then biscot_finish_overwrite_prompt(true); return true end
  if ch==K.LEFT or ch==K.RIGHT or ch==K.UP or ch==K.DOWN then
    overwrite_prompt_choice=(overwrite_prompt_choice==1) and 2 or 1
    speak(overwrite_prompt_choice==1 and "Yes" or "No")
    return true
  end
  if ch==K.ENTER then biscot_finish_overwrite_prompt(overwrite_prompt_choice==1); return true end
  local down=((gfx.mouse_cap or 0)&1)==1
  if down and not overwrite_prompt_mouse_was_down then
    local x,y=gfx.mouse_x or 0,gfx.mouse_y or 0
    if x>=90 and x<=240 and y>=48 and y<=84 then biscot_finish_overwrite_prompt(true); return true end
    if x>=500 and x<=650 and y>=48 and y<=84 then biscot_finish_overwrite_prompt(false); return true end
  end
  overwrite_prompt_mouse_was_down=down
  return true
end

function change_value(dir,big)
  local pat=ensure_pattern(current_pattern)
  local mult=jump_sizes[jump_index]*(big and 5 or 1)
  if is_header_row() then
    if current_cell==1 then
      local old=clampi(pat.length or 16,1,MAX_STEPS)
      local nv=clampi(old+dir*mult,1,MAX_STEPS)
      if nv==old then return end
      pat.length=nv
      refresh_pattern_after_edit(); speak_cell(); return
    elseif current_cell==2 then
      local old=clampi(pat.midi_note or -1,-1,127)
      local nv=clampi(old+dir*mult,-1,127)
      if nv==old then return end
      pat.midi_note=nv
      rebuild_midi_pattern_bank()
      refresh_pattern_after_edit(); speak_cell(); return
    elseif current_cell==3 then
      local old=step_recording_active and 1 or 0
      local nv=clampi(old+dir*mult,0,1)
      if nv==old then return end
      step_recording_active=(nv==1)
      step_record_sustain_down=false
      if step_recording_active then step_recording_step=clampi(last_real_step or 0,0,pattern_length()-1) end
      set_step_recording_midi_isolation(step_recording_active)
      refresh_pattern_after_edit(); speak_cell(); return
    else
      local old=clampi(pat.release_arp or 0,0,1)
      local nv=clampi(old+dir*mult,0,1)
      if nv==old then return end
      pat.release_arp=nv
      rebuild_midi_pattern_bank()
      refresh_pattern_after_edit(); speak_cell(); return
    end
  end
  if is_repeat_row() then
    local old=clampi(pat.repeat_count or 1,1,1000)
    local nv=clampi(old+dir*mult,1,1000)
    if nv==old then return end
    pat.repeat_count=nv
    if nv>old then
      -- New repeat groups inherit both values from the immediately preceding
      -- group, so extending Repeat continues the current timing/transpose.
      for i=old+1,nv do
        pat.repeat_speeds[i]=clamp(tonumber(pat.repeat_speeds[i-1]) or pat.speed or 1.0,0.25,16)
        pat.repeat_transposes[i]=clampi(pat.repeat_transposes[i-1] or 0,-96,96)
      end
    end
    for i=1,nv do
      if pat.repeat_speeds[i]==nil then
        pat.repeat_speeds[i]=i>1 and clamp(tonumber(pat.repeat_speeds[i-1]) or pat.speed or 1.0,0.25,16) or 1.0
      end
      if pat.repeat_transposes[i]==nil then
        pat.repeat_transposes[i]=i>1 and clampi(pat.repeat_transposes[i-1] or 0,-96,96) or 0
      end
    end
    refresh_pattern_after_edit(); speak_cell(); return
  end
  if is_speed_row() then
    local pair=math.floor((current_cell-1)/2)+1
    local ri=(speed_row_number()-1)*5+pair
    if current_cell%2==1 then
      local old=clamp(tonumber(pat.repeat_speeds[ri]) or pat.speed or 1.0,0.25,16)
      local nv=math.floor(clamp(old+dir*0.25*mult,0.25,16)*4+0.5)/4
      if math.abs(nv-old)<1e-12 then return end
      pat.repeat_speeds[ri]=nv
    else
      local old=clampi(pat.repeat_transposes[ri] or 0,-96,96)
      local nv=clampi(old+dir*mult,-96,96)
      if nv==old then return end
      pat.repeat_transposes[ri]=nv
    end
    refresh_pattern_after_edit(); speak_cell(); return
  end
  local c=current_cell_obj(); if not c then return end
  local p=PARAM_BY_ID[c.id]; if not p then return end
  local old=tonumber(c.value) or p.default
  local nv=clamp(old+dir*(tonumber(p.step) or 1)*mult,p.min,p.max)
  if p.tracker_curve then nv=math.floor(nv*100+0.5)/100 end
  if math.abs(nv-old)<1e-12 then return end
  c.value=nv
  refresh_pattern_after_edit()
  speak_cell()
  preview_pattern_step(current_step)
end
function add_param()
  local list=PARAM_LISTS[current_tab]
  local p=list[current_param_index]; if not p then return end
  if p.meta_pattern then
    current_step=-1; current_cell=2; logical_cell=1; page=1; speak_cell(); return
  end
  if not is_real_step() then speak("Select a pattern step") return end
  local cells=current_cells()
  local c,idx
  local special_added=false
  if p.allow_multiple then
    -- Sub Note is meaningful only after an active OP Arp Mode in the same
    -- chronological step row. It may be inserted more than once.
    local arp_index=nil
    for i,cell in ipairs(cells) do
      if cell.id==2022 and (tonumber(cell.value) or 0)>0 then arp_index=i end
    end
    if not arp_index then
      speak("Sub Note requires OP Arp Mode")
      return
    end
    local value=p.default
    table.insert(cells,{id=p.id,value=value})
    idx=#cells
    refresh_pattern_after_edit()
    speak((p.name or "Parameter").." added")
  else
    c,idx=find_cell(cells,p.id)
  end
  if not p.allow_multiple and not c and p.id==8 then
    -- Curve Resolution cannot act alone. If this row has no Curve yet, insert
    -- one immediately before it so the new command is usable at once.
    local curve_cell,curve_idx=find_cell(cells,4)
    if not curve_cell then
      -- Full Curve makes the newly inserted Resolution immediately audible.
      table.insert(cells,{id=4,value=100})
    end
    -- Resolution belongs immediately after Curve and before Curve Ratio, so
    -- both modifiers can be read naturally from left to right.
    local _,ratio_idx=find_cell(cells,9)
    if ratio_idx then
      table.insert(cells,ratio_idx,{id=8,value=p.default})
      idx=ratio_idx
    else
      table.insert(cells,{id=8,value=p.default})
      idx=#cells
    end
    refresh_pattern_after_edit()
    speak((p.name or "Parameter").." added")
    c=cells[idx]
    special_added=true
  end
  if not p.allow_multiple and not c and p.id==9 then
    -- Curve Ratio also requires Curve, but Resolution remains optional. At Off
    -- it changes nothing; the other modes interrupt the smooth Curve or modify
    -- the Resolution alternation for parameters placed to its right.
    local curve_cell=find_cell(cells,4)
    if not curve_cell then table.insert(cells,{id=4,value=100}) end
    table.insert(cells,{id=9,value=p.default})
    idx=#cells
    refresh_pattern_after_edit()
    speak((p.name or "Parameter").." added")
    c=cells[idx]
    special_added=true
  end
  if not p.allow_multiple and not c then
    local value=p.default
    -- Previous value is an ordered tracker command. Only parameters
    -- inserted after the marker inherit the same parameter from the previous step.
    local marker_index=nil
    for i,cell in ipairs(cells) do
      if cell.id==3 then marker_index=i end
    end
    if marker_index and p.id~=1 and p.id~=2 and p.id~=3 and p.id~=4 and p.id~=5 and p.id~=6 and p.id~=8 and p.id~=9 and current_step>0 then
      local prev_cells=ensure_pattern(current_pattern).steps[current_step-1][current_tab] or {}
      local prev=find_cell(prev_cells,p.id)
      if prev then value=prev.value end
    end
    table.insert(cells,{id=p.id,value=value})
    idx=#cells
    refresh_pattern_after_edit()
    speak((p.name or "Parameter").." added")
  elseif not p.allow_multiple and not special_added then
    speak((p.name or "Parameter").." already present")
  end
  last_added_param_index[current_tab]=current_param_index
  logical_cell=idx or 1
  current_cell=logical_cell
end
function copy_current_parameter()
  if not is_real_step() then return end
  local c=current_cell_obj()
  if not c then return end
  cell_clipboard={id=c.id,value=c.value}
  pattern_clipboard_mode="parameter"
  clipboard_enter_pending=true
  clipboard_pattern=current_pattern
  clipboard_step=current_step
  clipboard_tab=current_tab
  local p=PARAM_BY_ID[c.id]
  speak("Copied "..(p and p.name or "Parameter")..", "..value_text(c))
end
function cut_current_parameter()
  if not is_real_step() then return end
  local cells=current_cells()
  local c=current_cell_obj()
  if not c or #cells==0 then return end
  local p=PARAM_BY_ID[c.id]
  local name=p and p.name or "Parameter"
  local spoken_value=value_text(c)
  cell_clipboard={id=c.id,value=c.value}
  pattern_clipboard_mode="parameter"
  clipboard_enter_pending=false
  clipboard_pattern=current_pattern
  clipboard_step=current_step
  clipboard_tab=current_tab
  table.remove(cells,current_cell)
  logical_cell=clampi(logical_cell,1,math.max(1,#cells))
  current_cell=logical_cell
  refresh_pattern_after_edit()
  speak("Cut "..name..", "..spoken_value)
end

function paste_current_parameter()
  if not is_real_step() or not cell_clipboard then return end
  if clipboard_pattern==current_pattern and clipboard_step==current_step and clipboard_tab==current_tab then return end
  local cells=current_cells()
  local existing,existing_index=find_cell(cells,cell_clipboard.id)
  if existing then return end
  if #cells>=MAX_CELLS then return end
  local insert_at=clampi(current_cell,1,#cells+1)
  table.insert(cells,insert_at,{id=cell_clipboard.id,value=cell_clipboard.value})
  current_cell=insert_at
  logical_cell=insert_at
  refresh_pattern_after_edit()
  local p=PARAM_BY_ID[cell_clipboard.id]
  speak("Pasted "..(p and p.name or "Parameter")..", "..value_text(cells[insert_at]))
end
function controller_window_title()
  if page==5 then
    local label
    if f5_virtual_root then
      label="Library"
    else
      local root=norm_path(f5_scope_root~="" and f5_scope_root or f5_root_dir())
      local cur=norm_path(f5_path~="" and f5_path or root)
      if cur==root and f5_scope_label and f5_scope_label~="" then
        label=f5_scope_label
      else
        label=base_name(cur)
      end
      if not label or label=="" then label="Library" end
    end
    return label..". Biscot1 TrackerControl"
  elseif page==6 then
    return "Diagnostics. Biscot1 TrackerControl"
  elseif page==3 or page==4 then
    local n=song_names[current_song]
    return "Song "..tostring(current_song)..(n and (". "..n) or "")..". Biscot1 TrackerControl"
  else
    local n=pattern_names[current_pattern]
    return "Pattern "..tostring(current_pattern)..(n and (". "..n) or "")..". Biscot1 TrackerControl"
  end
end

restore_controller_focus_after_dialog=function()
  ignore_enter_until=(reaper.time_precise and reaper.time_precise() or 0)+0.55
  local title=controller_window_title()
  local tries=0
  local function refocus()
    tries=tries+1
    local hwnd=nil
    if reaper.JS_Window_Find then
      hwnd=reaper.JS_Window_Find(title,true)
      if not hwnd then hwnd=reaper.JS_Window_Find("Biscot1 TrackerControl",false) end
      if hwnd then
        if reaper.JS_Window_SetForeground then reaper.JS_Window_SetForeground(hwnd) end
        if reaper.JS_Window_SetFocus then reaper.JS_Window_SetFocus(hwnd) end
      end
    elseif reaper.BR_Win32_FindWindowEx and reaper.BR_Win32_SetFocus then
      hwnd=reaper.BR_Win32_FindWindowEx(nil,nil,nil,title)
      if hwnd then reaper.BR_Win32_SetFocus(hwnd) end
    end
    if not hwnd and tries<4 then reaper.defer(refocus) end
  end
  reaper.defer(refocus)
end

function paste_copied_parameter_to_next_steps()
  if not cell_clipboard then return end
  if clipboard_pattern~=current_pattern then return end
  if clipboard_step==nil or clipboard_step<0 then return end

  local pat=ensure_pattern(current_pattern)
  local last_step=clampi(pat.length or 16,1,MAX_STEPS)-1
  if clipboard_step>=last_step then
    speak("No following pattern steps")
    return
  end

  local available=last_step-clipboard_step
  local ok,text=reaper.GetUserInputs("Copy parameter to following steps",1,"Number of following steps:",tostring(math.min(8,available)))
  restore_controller_focus_after_dialog()
  if not ok then return end
  local requested=tonumber(tostring(text or ""):match("^%s*(%-?%d+)%s*$"))
  if not requested or requested<1 then
    speak("Invalid number of steps")
    return
  end
  requested=clampi(requested,1,available)

  local copied=0
  local skipped=0
  local source_position=1
  local source_cells=pat.steps[clipboard_step][clipboard_tab] or {}
  local _,source_index=find_cell(source_cells,cell_clipboard.id)
  if source_index then source_position=source_index end

  for st=clipboard_step+1,clipboard_step+requested do
    local cells=pat.steps[st][clipboard_tab]
    local existing=find_cell(cells,cell_clipboard.id)
    if existing then
      skipped=skipped+1
    elseif #cells<MAX_CELLS then
      local insert_at=clampi(source_position,1,#cells+1)
      table.insert(cells,insert_at,{id=cell_clipboard.id,value=cell_clipboard.value})
      copied=copied+1
    else
      skipped=skipped+1
    end
  end

  if copied>0 then refresh_pattern_after_edit() end
  local param=PARAM_BY_ID[cell_clipboard.id]
  local name=param and param.name or "Parameter"
  if skipped>0 then
    speak(name.." copied to "..copied.." following steps. "..skipped.." skipped")
  else
    speak(name.." copied to "..copied.." following steps")
  end
end
function set_jump_index(new_index)
  new_index=clampi(new_index,1,#jump_sizes)
  if new_index==jump_index then return end
  jump_index=new_index
  speak("Value step "..jump_sizes[jump_index])
end
function move_pattern_step(new_step)
  local last=pattern_length()+repeat_speed_row_count()
  new_step=clampi(new_step,-1,last)
  if new_step==current_step then return end
  current_step=new_step
  if current_step>=0 and current_step<pattern_length() then last_real_step=current_step end
  if is_real_step() then
    local cells=current_cells()
    current_cell=(#cells>0) and clampi(logical_cell,1,#cells) or 1
  elseif is_header_row() then
    current_cell=1
  else
    current_cell=clampi(current_cell,1,meta_cell_count())
  end
  speak_step_cell()
  if is_real_step() then preview_pattern_step(current_step) end
end
function move_pattern_cell(new_cell)
  if is_real_step() then
    local maxc=math.max(1,#current_cells())
    new_cell=clampi(new_cell,1,maxc)
    if new_cell==current_cell and logical_cell==new_cell then return end
    logical_cell=new_cell; current_cell=new_cell; speak_cell(); return
  end
  local maxc=meta_cell_count()
  if maxc<=0 then return end
  new_cell=clampi(new_cell,1,maxc)
  if new_cell==current_cell then return end
  current_cell=new_cell; speak_cell()
end
function update_controller_title()
  local title=controller_window_title()
  local w=math.max(320,gfx.w or 760)
  local h=math.max(80,gfx.h or 100)
  local x=gfx.x or 0
  local y=gfx.y or 0
  gfx.init(title,w,h,0,x,y)
end

function change_current_song(delta)
  local old_lane=song_lane
  local old_cell=song_cell
  local old_speed_index=is_song_speed_row() and song_speed_index() or nil
  local section
  if song_lane==4 then section="global"
  elseif song_step>=0 and song_step<song_lane_length(song_lane) then section="steps"
  elseif song_step==song_lane_length(song_lane) then section="repeat"
  elseif is_song_speed_row() then section="speed"
  else section="length" end

  local target=clampi(current_song+delta,0,MAX_SONGS-1)
  if target==current_song then return end
  if song_playing then song_playing=false; write_song_memory(false) end
  -- Commit the Song we are leaving before changing the active object. Without
  -- this, Song End could remain only in the old Lua object and never reach its
  -- indexed GMEM slot or the project state.
  songs[current_song]=song
  song._song_index=current_song
  reaper.gmem_attach(GMEM_NS)
  write_one_song_memory(song,SONG_BANK_BASE+current_song*SONG_BANK_STRIDE)
  rebuild_song_midi_map()
  save_song_project()
  set_active_song(target)
  ensure_song_lane_meta(song)
  ensure_song_mixer(song)
  clear_song_step_selection()
  load_song_mixer_monitor_state(song); song_mixer_lane=1

  if section=="global" then
    song_lane=4; song_step=-1
    local maxc=song_global_cell_count()
    song_cell=clampi(old_cell,1,maxc)
  else
    song_lane=clampi(old_lane,1,3)
    if section=="steps" then
      song_step=0; song_cell=1
    elseif section=="repeat" then
      song_step=song_lane_length(song_lane); song_cell=1
    elseif section=="speed" then
      local rc=song_lane_repeat_count(song_lane)
      local idx=clampi(old_speed_index or 1,1,rc)
      song_step=song_lane_length(song_lane)+math.floor((idx-1)/10)+1
      song_cell=((idx-1)%10)+1
    else
      song_step=-1; song_cell=1
    end
  end

  save_song_project(); write_song_memory(false); update_controller_title()
  speak("Song "..current_song)
end

function change_current_pattern(delta)
  local old_cell=current_cell
  local old_speed_index=nil
  local old_speed_kind=nil
  local section
  if is_real_step() then
    section="steps"
  elseif is_repeat_row() then
    section="repeat"
  elseif is_speed_row() then
    section="speed"
    local pair=math.floor((current_cell-1)/2)+1
    old_speed_index=(speed_row_number()-1)*5+pair
    old_speed_kind=(current_cell%2==1) and 1 or 2
  else
    section="length"
  end

  local target=clampi(current_pattern+delta,0,MAX_PATTERNS-1)
  if target==current_pattern then return end
  if playing then playing=false; push_pattern(false) end
  activate_pattern(target)
  update_controller_title()

  if section=="steps" then
    current_step=0; current_cell=1; logical_cell=1
  elseif section=="repeat" then
    current_step=pattern_length(); current_cell=1; logical_cell=1
  elseif section=="speed" then
    local pat=ensure_pattern(current_pattern)
    local rc=clampi(pat.repeat_count or 1,1,1000)
    local idx=clampi(old_speed_index or 1,1,rc)
    current_step=pattern_length()+math.floor((idx-1)/5)+1
    current_cell=((idx-1)%5)*2+(old_speed_kind or 1)
    logical_cell=1
  else
    current_step=-1; current_cell=clampi(old_cell,1,meta_cell_count()); logical_cell=1
  end

  write_text_file(active_pattern_path(),tostring(current_pattern))
  write_text_file(active_pattern_file_path(),"")
  if reaper.SetProjExtState then reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"active_pattern_file","") end
  write_pattern_memory()
  save_ram_pattern_snapshot()
  speak("Pattern "..tostring(current_pattern))
end

function jump_to_pattern_number()
  local ok,text=reaper.GetUserInputs("Pattern "..tostring(current_pattern)..". Biscot1 TrackerControl",1,"Pattern 0 to 999:",tostring(current_pattern))
  restore_controller_focus_after_dialog()
  if not ok then return end
  local raw=tostring(text or ""):match("^%s*(%d+)%s*$")
  local target=raw and tonumber(raw) or nil
  if target==nil or target<0 or target>MAX_PATTERNS-1 then
    speak("Invalid pattern number")
    return
  end
  target=clampi(target,0,MAX_PATTERNS-1)
  if target==current_pattern then return end
  if playing then playing=false; push_pattern(false) end
  activate_pattern(target)
  update_controller_title()
  -- A real pattern change always starts from the Length header cell.
  -- Cursor memory remains active only while staying inside the same pattern.
  current_step=-1
  current_cell=1
  logical_cell=1
  write_text_file(active_pattern_path(),tostring(current_pattern))
  write_text_file(active_pattern_file_path(),"")
  if reaper.SetProjExtState then reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"active_pattern_file","") end
  write_pattern_memory()
  save_ram_pattern_snapshot()
end

function jump_to_song_number()
  local ok,text=reaper.GetUserInputs("Song "..tostring(current_song)..". Biscot1 TrackerControl",1,"Song 0 to 99:",tostring(current_song))
  restore_controller_focus_after_dialog()
  if not ok then return end
  local raw=tostring(text or ""):match("^%s*(%d+)%s*$")
  local target=raw and tonumber(raw) or nil
  if target==nil or target<0 or target>MAX_SONGS-1 then
    speak("Invalid song number")
    return
  end
  target=clampi(target,0,MAX_SONGS-1)
  if target==current_song then return end
  if song_playing then song_playing=false; write_song_memory(false) end
  -- Commit the Song we are leaving before changing the active object. Without
  -- this, Song End could remain only in the old Lua object and never reach its
  -- indexed GMEM slot or the project state.
  songs[current_song]=song
  song._song_index=current_song
  reaper.gmem_attach(GMEM_NS)
  write_one_song_memory(song,SONG_BANK_BASE+current_song*SONG_BANK_STRIDE)
  rebuild_song_midi_map()
  save_song_project()
  set_active_song(target)
  ensure_song_mixer(song)
  clear_song_step_selection()
  load_song_mixer_monitor_state(song); song_mixer_lane=1
  song_step=-1; song_cell=1; song_lane=1
  save_song_project(); write_song_memory(false)
  update_controller_title()
  speak("Song "..current_song)
end


function jump_to_pattern_step()
  local max_step=math.max(0,pattern_length()-1)
  local initial=is_real_step() and current_step or clampi(last_real_step or 0,0,max_step)
  local ok,text=reaper.GetUserInputs("Go to Pattern Step",1,"Step 0 to "..tostring(max_step)..":",tostring(initial))
  restore_controller_focus_after_dialog()
  if not ok then return end
  local raw=tostring(text or ""):match("^%s*(%d+)%s*$")
  local target=raw and tonumber(raw) or nil
  if target==nil or target<0 or target>max_step then
    speak("Invalid step number")
    return
  end
  clear_pattern_step_selection()
  current_step=clampi(target,0,max_step)
  last_real_step=current_step
  local n=#current_cells()
  current_cell=n>0 and clampi(logical_cell,1,n) or 1
  update_controller_title()
  local announced_step=current_step
  local announce_tries=0
  local function announce_after_refocus()
    announce_tries=announce_tries+1
    if announce_tries<8 then
      reaper.defer(announce_after_refocus)
    elseif page==1 and current_step==announced_step then
      speak_step_cell()
    end
  end
  reaper.defer(announce_after_refocus)
  preview_pattern_step(current_step)
end

function jump_to_song_step()
  local max_step=math.max(0,song_lane_length(song_lane)-1)
  local initial=(song_step>=0 and song_step<=max_step) and song_step or 0
  local ok,text=reaper.GetUserInputs("Go to Song Step",1,"Step 0 to "..tostring(max_step)..":",tostring(initial))
  restore_controller_focus_after_dialog()
  if not ok then return end
  local raw=tostring(text or ""):match("^%s*(%d+)%s*$")
  local target=raw and tonumber(raw) or nil
  if target==nil or target<0 or target>max_step then
    speak("Invalid step number")
    return
  end
  local preferred_param,preferred_cell=song_cursor_preference()
  clear_song_step_selection()
  song_step=clampi(target,0,max_step)
  song_info_step_by_lane[song_lane]=song_step
  restore_song_cursor_preference(preferred_param,preferred_cell)
  update_controller_title()
  local announced_step=song_step
  local announce_tries=0
  local function announce_after_refocus()
    announce_tries=announce_tries+1
    if announce_tries<8 then
      reaper.defer(announce_after_refocus)
    elseif page==3 and song_step==announced_step then
      speak_song_position(true)
    end
  end
  reaper.defer(announce_after_refocus)
  preview_song_row_pattern()
end

function reorder_current_parameter(dir)
  if not is_real_step() then return end
  local cells=current_cells()
  if #cells<2 then return end
  local from=clampi(current_cell,1,#cells)
  local to=from+dir
  if to<1 or to>#cells then return end
  cells[from],cells[to]=cells[to],cells[from]
  current_cell=to
  logical_cell=to
  refresh_pattern_after_edit()
  speak_cell()
end

function move_pattern_vertical_bounded(delta)
  local target
  if is_header_row() or is_real_step() then
    -- Length and Steps form one open vertical range.
    -- Up from Step 0 reaches Length; Down from Length enters Step 0.
    -- Arrow and Page navigation never pass below the last real Step.
    target=clampi(current_step+delta,-1,pattern_length()-1)
  else
    -- Repeat field includes Repeat plus all Speed/Transpose rows.
    target=clampi(current_step+delta,pattern_length(),pattern_length()+repeat_speed_row_count())
  end
  move_pattern_step(target)
end

function move_song_vertical_bounded(delta)
  if song_lane==4 then return end
  local preferred_param,preferred_cell=song_cursor_preference()
  local target=song_step
  local old_speed=is_song_speed_row()
  local old_cell=song_cell
  local lane_len=song_lane_length(song_lane)
  if song_step<0 or (song_step>=0 and song_step<lane_len) then
    target=clampi(song_step+delta,-1,math.max(0,lane_len-1))
  elseif song_step>=lane_len then
    target=clampi(song_step+delta,lane_len,lane_len+song_speed_row_count())
  end
  if target~=song_step then
    song_step=target
    if song_step>=0 and song_step<song_lane_length(song_lane) then
      song_info_step_by_lane[song_lane]=song_step
      restore_song_cursor_preference(preferred_param,preferred_cell)
    elseif old_speed and is_song_speed_row() then
      song_cell=clampi(old_cell,1,math.max(1,song_speed_cell_count()))
    else
      song_cell=1
    end
    speak_song_position(true)
    if song_step>=0 and song_step<song_lane_length(song_lane) then preview_song_row_pattern() end
  end
end

function pattern_step_has_content_at(st)
  if st<0 or st>=pattern_length() then return false end
  local pat=ensure_pattern(current_pattern)
  for tab=1,3 do
    if #(pat.steps[st][tab] or {})>0 then return true end
  end
  return false
end
function jump_pattern_nonempty(dir)
  if not is_real_step() then return false end
  local st=current_step+dir
  local last=pattern_length()-1
  while st>=0 and st<=last do
    if pattern_step_has_content_at(st) then
      clear_pattern_step_selection()
      move_pattern_step(st)
      return true
    end
    st=st+dir
  end
  return false
end

function pattern_transport_step(delta)
  local len=pattern_length()
  local base=is_real_step() and current_step or clampi(last_real_step or 0,0,len-1)
  local target=clampi(base+delta,0,len-1)
  if target==base then return end
  clear_pattern_step_selection()
  current_step=target
  last_real_step=target
  current_cell=1
  logical_cell=1
  local cells=current_cells()
  if #cells>0 then current_cell=1; logical_cell=1 end
  speak_step_cell()
  if playing and not f5_preview_active then
    -- During playback the transport jump is immediate, regardless of P mode.
    -- The tracker restores the complete parameter history before the row.
    push_pattern(true,target)
  end
end

function select_pattern_section(tab)
  tab=clampi(tab,1,3)
  clear_pattern_step_selection()
  if is_real_step() then last_pattern_cell_by_tab[current_tab]=logical_cell end
  current_tab=tab
  if is_real_step() then
    logical_cell=last_pattern_cell_by_tab[current_tab] or 1
    local n=#current_cells()
    current_cell=n>0 and clampi(logical_cell,1,n) or 1
  else
    current_cell=1
  end
  speak_position()
end

function return_from_pattern_to_song()
  local r=song_pattern_return_context
  if not r then return false end
  song_pattern_return_context=nil
  set_active_song(clampi(r.song or current_song,0,MAX_SONGS-1))
  ensure_song_lane_meta(song)
  ensure_song_mixer(song)
  song_mixer_lane=clampi(r.mixer_lane or r.lane or 1,1,3)
  song_lane=clampi(r.lane or 1,1,4)
  if song_lane==4 then
    song_step=-1
    song_cell=clampi(r.cell or 1,1,song_global_cell_count())
  else
    local len=song_lane_length(song_lane)
    local maxrow=len+song_speed_row_count()
    song_step=clampi(r.step or 0,-1,maxrow)
    song_cell=math.max(1,r.cell or 1)
    song_logical_param=r.logical_param
    song_logical_cell=math.max(1,r.logical_cell or r.cell or 1)
    if song_step>=0 and song_step<len then
      restore_song_cursor_preference(song_logical_param,song_logical_cell)
      song_info_step_by_lane[song_lane]=song_step
    end
  end
  song_last_operator_lane=(song_lane>=1 and song_lane<=3) and song_lane or song_mixer_lane
  biscot_change_page(3)
  speak_song_position()
  return true
end

function handle_f1(ch)
  if not ctrl() and not alt() and not shift() and (ch==string.byte("d") or ch==string.byte("D")) then toggle_core_model(); return end
  if not ctrl() and not alt() and not shift() and (ch==string.byte("o") or ch==string.byte("O")) then toggle_core_audio_mode(); return end
  if not ctrl() and not alt() and not shift() and (ch==string.byte("1") or ch==string.byte("2") or ch==string.byte("3")) then
    select_pattern_section(ch-string.byte("0"))
    return
  end
  if not ctrl() and not alt() and not shift() and (ch==string.byte("p") or ch==string.byte("P")) then
    pattern_play_from_cursor=not pattern_play_from_cursor
    speak(pattern_play_from_cursor and "Play from current step" or "Play from beginning")
    return
  end
  if ctrl() and not alt() and not shift() and ch==K.CTRL_J then jump_to_pattern_step(); return end
  if ctrl() and not alt() and not shift() and ch==K.CTRL_Z then biscot_undo_pattern(); return end
  if shift() and not ctrl() and not alt() and ch==K.UP then extend_pattern_step_selection(-1); return end
  if shift() and not ctrl() and not alt() and ch==K.DOWN then extend_pattern_step_selection(1); return end
  if shift() and not ctrl() and not alt() and ch==K.PAGE_UP then extend_pattern_step_selection(-math.max(5,jump_sizes[jump_index])); return end
  if shift() and not ctrl() and not alt() and ch==K.PAGE_DOWN then extend_pattern_step_selection(math.max(5,jump_sizes[jump_index])); return end
  if ctrl() and shift() and alt() and (ch==K.CTRL_C or ch==string.byte("C") or ch==string.byte("c") or (gfx.getchar and (gfx.getchar(string.byte("C"))>0 or gfx.getchar(string.byte("c"))>0))) then
    copy_entire_current_pattern()
    return
  end
  if ctrl() and ch==K.CTRL_S then
    local ok,path=save_pattern_with_name()
    if ok then
      refresh_f5_entries(base_name(path))
      speak("Saved "..base_name(path):gsub("%.txt$",""))
    elseif path~="" then
      speak("Pattern save failed")
    end
    return
  end
  if ctrl() and ch==K.TAB then
    -- Only a Pattern opened from F3 owns a return destination.  Otherwise
    -- Ctrl+Tab is deliberately inert and never cycles OP/Filter/LFO sections.
    return_from_pattern_to_song()
    return
  end
  if ch==K.TAB then speak_tab_position(); return end
  if (ch==K.BACKSPACE or ch==K.BACKSPACE2) then
    if ctrl() and not alt() and not shift() then
      if reset_core_to_defaults() then speak("Biscot1 reset") end
    elseif not ctrl() and not alt() and not shift() then
      reset_current_parameter_to_default()
    end
    return
  end
  if ctrl() and not alt() and not shift() and ch==K.UP then clear_pattern_step_selection(); change_current_pattern(-1); return end
  if ctrl() and not alt() and not shift() and ch==K.DOWN then clear_pattern_step_selection(); change_current_pattern(1); return end
  if ctrl() and shift() and not alt() and ch==K.UP then pattern_transport_step(-1); return end
  if ctrl() and shift() and not alt() and ch==K.DOWN then pattern_transport_step(1); return end
  if ctrl() and not alt() and not shift() and ch==K.LEFT then reorder_current_parameter(-1); return end
  if ctrl() and not alt() and not shift() and ch==K.RIGHT then reorder_current_parameter(1); return end
  if ctrl() and ch==K.CTRL_X then
    if pattern_step_selection_active() then cut_selected_pattern_steps() else cut_current_parameter() end
    return
  end
  if ctrl() and ch==K.CTRL_C then
    if pattern_step_selection_active() then copy_selected_pattern_steps() else copy_current_parameter() end
    return
  end
  if ctrl() and ch==K.CTRL_V then
    -- Ctrl+V follows the most recent copy operation.  A stale full-Pattern TXT
    -- in the operating-system clipboard must never override a later step or
    -- parameter copy made inside the tracker.
    if pattern_clipboard_mode=="steps" then
      paste_selected_pattern_steps()
    elseif pattern_clipboard_mode=="parameter" then
      paste_current_parameter()
    elseif pattern_clipboard_mode=="full_pattern" then
      paste_entire_pattern()
    else
      -- Allow an externally supplied Biscot1 Pattern TXT only when no internal
      -- copy mode has been selected in this controller session.
      local cliptxt=biscot_get_text_from_clipboard()
      if cliptxt and cliptxt:find("BISCOT1_TRACKER_RAM_V2",1,true) then
        whole_pattern_clipboard=cliptxt
        pattern_clipboard_mode="full_pattern"
        paste_entire_pattern()
      end
    end
    return
  end
  if ch==K.ENTER then
    local now=reaper.time_precise and reaper.time_precise() or 0
    if now<ignore_enter_until then return end
    if clipboard_enter_pending and cell_clipboard and clipboard_pattern==current_pattern then
      clipboard_enter_pending=false
      paste_copied_parameter_to_next_steps()
    else
      jump_to_pattern_number()
    end
    return
  end
  if ch==K.SPACE then
    if playing and not f5_preview_active then
      playing=false
      push_pattern(false)
    else
      biscot_prepare_preview("pattern")
      playing=true
      local start_step=0
      if pattern_play_from_cursor then
        start_step=is_real_step() and current_step or clampi(last_real_step or 0,0,math.max(0,pattern_length()-1))
      end
      push_pattern(true,start_step)
    end
    return
  end
  if alt() and ch==K.LEFT then set_jump_index(jump_index-1); return end
  if alt() and ch==K.RIGHT then set_jump_index(jump_index+1); return end
  if alt() and ch==K.UP then change_value(1,false); return end
  if alt() and ch==K.DOWN then change_value(-1,false); return end
  if alt() and ch==K.PAGE_UP then change_value(1,true); return end
  if alt() and ch==K.PAGE_DOWN then change_value(-1,true); return end
  -- As in the working LS-101 controller: Alt+Home = minimum, Alt+End = maximum.
  if alt() and ch==K.HOME then
    local pat=ensure_pattern(current_pattern)
    if is_header_row() then
      if current_cell==1 then if pat.length~=1 then pat.length=1; refresh_pattern_after_edit(); speak_cell() end
      elseif current_cell==2 then if (pat.midi_note or -1)~=-1 then pat.midi_note=-1; rebuild_midi_pattern_bank(); refresh_pattern_after_edit(); speak_cell() end
      elseif current_cell==3 then if step_recording_active then step_recording_active=false; step_record_sustain_down=false; set_step_recording_midi_isolation(false); refresh_pattern_after_edit(); speak_cell() end
      else if (pat.release_arp or 0)~=0 then pat.release_arp=0; rebuild_midi_pattern_bank(); refresh_pattern_after_edit(); speak_cell() end end
    elseif is_repeat_row() then
      if pat.repeat_count~=1 then pat.repeat_count=1; current_cell=1; refresh_pattern_after_edit(); speak_cell() end
    elseif is_speed_row() then
      local pair=math.floor((current_cell-1)/2)+1; local ri=(speed_row_number()-1)*5+pair
      if current_cell%2==1 then
        if pat.repeat_speeds[ri]~=0.25 then pat.repeat_speeds[ri]=0.25; refresh_pattern_after_edit(); speak_cell() end
      else
        if pat.repeat_transposes[ri]~=-96 then pat.repeat_transposes[ri]=-96; refresh_pattern_after_edit(); speak_cell() end
      end
    else local c=current_cell_obj(); if c then local p=PARAM_BY_ID[c.id]; local old=c.value; c.value=p.min; if c.value~=old then refresh_pattern_after_edit(); speak_cell() end end end
    return
  end
  if alt() and ch==K.ENDK then
    local pat=ensure_pattern(current_pattern)
    if is_header_row() then
      if current_cell==1 then if pat.length~=MAX_STEPS then pat.length=MAX_STEPS; refresh_pattern_after_edit(); speak_cell() end
      elseif current_cell==2 then if (pat.midi_note or -1)~=127 then pat.midi_note=127; rebuild_midi_pattern_bank(); refresh_pattern_after_edit(); speak_cell() end
      elseif current_cell==3 then if not step_recording_active then step_recording_active=true; step_recording_step=clampi(last_real_step or 0,0,pattern_length()-1); step_record_sustain_down=false; set_step_recording_midi_isolation(true); refresh_pattern_after_edit(); speak_cell() end
      else if (pat.release_arp or 0)~=1 then pat.release_arp=1; rebuild_midi_pattern_bank(); refresh_pattern_after_edit(); speak_cell() end end
    elseif is_repeat_row() then
      if pat.repeat_count~=1000 then pat.repeat_count=1000; for i=1,1000 do if pat.repeat_speeds[i]==nil then pat.repeat_speeds[i]=1.0 end; if pat.repeat_transposes[i]==nil then pat.repeat_transposes[i]=0 end end; refresh_pattern_after_edit(); speak_cell() end
    elseif is_speed_row() then
      local pair=math.floor((current_cell-1)/2)+1; local ri=(speed_row_number()-1)*5+pair
      if current_cell%2==1 then
        if pat.repeat_speeds[ri]~=16 then pat.repeat_speeds[ri]=16; refresh_pattern_after_edit(); speak_cell() end
      else
        if pat.repeat_transposes[ri]~=96 then pat.repeat_transposes[ri]=96; refresh_pattern_after_edit(); speak_cell() end
      end
    else local c=current_cell_obj(); if c then local p=PARAM_BY_ID[c.id]; local old=c.value; c.value=p.max; if c.value~=old then refresh_pattern_after_edit(); speak_cell() end end end
    return
  end
  local pg=math.max(5,jump_sizes[jump_index])
  if ctrl() and not alt() and not shift() and ch==K.PAGE_UP then jump_pattern_nonempty(-1); return end
  if ctrl() and not alt() and not shift() and ch==K.PAGE_DOWN then jump_pattern_nonempty(1); return end
  if ch==K.UP then
    if pattern_plain_arrow_is_shift_echo(-1) then return end
    clear_pattern_step_selection()
    move_pattern_vertical_bounded(-1)
    return
  end
  if ch==K.DOWN then
    if pattern_plain_arrow_is_shift_echo(1) then return end
    clear_pattern_step_selection()
    move_pattern_vertical_bounded(1)
    return
  end
  if ch==K.PAGE_UP then
    if pattern_plain_arrow_is_shift_echo(-pg) then return end
    clear_pattern_step_selection(); move_pattern_vertical_bounded(-pg); return
  end
  if ch==K.PAGE_DOWN then
    if pattern_plain_arrow_is_shift_echo(pg) then return end
    clear_pattern_step_selection(); move_pattern_vertical_bounded(pg); return
  end
  if ch==K.HOME then
    clear_pattern_step_selection()
    -- Non-cyclic forward movement through the main pattern sections:
    -- Length -> first step -> Repeat -> first Speed. Silent at the Speed edge.
    if is_header_row() then
      move_pattern_step(0)
    elseif is_real_step() then
      move_pattern_step(pattern_length())
    elseif is_repeat_row() then
      move_pattern_step(pattern_length()+1)
    end
    return
  end
  if ch==K.ENDK then
    clear_pattern_step_selection()
    -- Non-cyclic backward movement through the main pattern sections:
    -- Speed -> Repeat -> last step -> Length. Silent at the Length edge.
    if is_speed_row() then
      move_pattern_step(pattern_length())
    elseif is_repeat_row() then
      move_pattern_step(pattern_length()-1)
    elseif is_real_step() then
      move_pattern_step(-1)
    end
    return
  end
  if ch==K.LEFT then move_pattern_cell(current_cell-1); return end
  if ch==K.RIGHT then move_pattern_cell(current_cell+1); return end
  if ch==K.DELETE then
    if ctrl() and alt() and shift() then reset_entire_current_pattern(); return end
    if ctrl() and shift() and not alt() then clear_current_pattern_table(); return end
    if ctrl() and not shift() and not alt() then reset_current_step_table(); return end
    if not ctrl() and not shift() and not alt() then
      if pattern_step_selection_active() then
        delete_selected_pattern_steps()
        return
      end
      if not is_real_step() then return end
      local cells=current_cells()
      if #cells>0 then
        table.remove(cells,current_cell)
        logical_cell=clampi(logical_cell,1,math.max(1,#cells))
        current_cell=logical_cell
        refresh_pattern_after_edit()
        speak("deleted")
      end
      return
    end
  end
end
function f2_row(pos) return ((pos-1)%param_grid_height)+1 end
function f2_col(pos) return math.floor((pos-1)/param_grid_height)+1 end
function f2_speak()
  local p=PARAM_LISTS[current_tab][current_param_index]
  if p then speak(p.name) end
end
function f2_move(target)
  local list=PARAM_LISTS[current_tab]
  target=clampi(target,1,#list)
  if target==current_param_index then return end
  current_param_index=target; f2_speak()
end
function parameter_name_matches_letter(name,wanted)
  name=tostring(name or ""):lower()
  if name:sub(1,1)==wanted then return true end
  -- Parameter lists often use structural prefixes such as OP, Filter and LFO.
  -- Letter navigation must also reach the meaningful words: C finds Filter
  -- Cutoff, R finds Filter Resonance, W finds OP Wave, and so on.
  for word in name:gmatch("[%w]+") do
    if word:sub(1,1)==wanted and word~="op" and word~="filter" and not word:match("^lfo%d*$") then return true end
  end
  return false
end
function parameter_jump_to_initial(ch,list,current_index,move_fn)
  if ctrl() or shift() or alt() then return false end
  if ch<48 or (ch>57 and ch<65) or (ch>90 and ch<97) or ch>122 then return false end
  local wanted=string.char(ch):lower()
  local count=#list
  if count<1 then return true end
  for offset=1,count do
    local idx=((current_index-1+offset)%count)+1
    local item=list[idx]
    local name=type(item)=="table" and tostring(item.name or "") or tostring(item or "")
    if parameter_name_matches_letter(name,wanted) then move_fn(idx); return true end
  end
  return true
end

function handle_f2(ch)
  if not is_real_step() then biscot_change_page(1); speak_position(); return end
  local list=PARAM_LISTS[current_tab]
  if parameter_jump_to_initial(ch,list,current_param_index,f2_move) then return end
  if ctrl() and ch==K.TAB then
    last_added_param_index[current_tab]=current_param_index
    current_tab=current_tab%3+1
    local newlist=PARAM_LISTS[current_tab]
    current_param_index=clampi(last_added_param_index[current_tab] or 1,1,math.max(1,#newlist))
    local p=newlist[current_param_index]
    speak("Parameters "..TAB_NAMES[current_tab]..". Grid "..param_grid_height..". "..(p and p.name or "empty"))
    return
  end
  if ctrl() and ch==K.LEFT then
    if param_grid_height<=1 then return end
    param_grid_height=param_grid_height-1; current_param_index=clampi(current_param_index,1,#list); speak("Grid "..param_grid_height); return
  end
  if ctrl() and ch==K.RIGHT then
    if param_grid_height>=100 then return end
    param_grid_height=param_grid_height+1; current_param_index=clampi(current_param_index,1,#list); speak("Grid "..param_grid_height); return
  end
  local row=f2_row(current_param_index)
  local col=f2_col(current_param_index)
  local col_first=(col-1)*param_grid_height+1
  local col_last=math.min(#list,col*param_grid_height)

  -- Vertical movement never leaves the current column.
  if ch==K.UP then
    if row<=1 then return end
    f2_move(current_param_index-1); return
  end
  if ch==K.DOWN then
    if current_param_index>=col_last then return end
    f2_move(current_param_index+1); return
  end

  -- Horizontal movement keeps the same row. A missing cell is a silent border.
  if ch==K.LEFT then
    if col<=1 then return end
    local target=current_param_index-param_grid_height
    if target<1 then return end
    f2_move(target); return
  end
  if ch==K.RIGHT then
    local target=current_param_index+param_grid_height
    if target>#list then return end
    f2_move(target); return
  end

  -- Ctrl+Home/End address the whole parameter grid.
  if ctrl() and ch==K.HOME then f2_move(#list); return end
  if ctrl() and ch==K.ENDK then f2_move(1); return end

  -- Home/End stay inside the current column.
  if ch==K.HOME then f2_move(col_last); return end
  if ch==K.ENDK then f2_move(col_first); return end

  -- Page movement follows F1 and uses the selected jump size, defaulting to five.
  local pg=math.max(5,jump_sizes[jump_index])
  if ch==K.PAGE_UP then f2_move(current_param_index-pg); return end
  if ch==K.PAGE_DOWN then f2_move(current_param_index+pg); return end

  -- Ctrl+Enter adds and stays in F2. Enter adds, then returns to the current F1 step.
  if ch==K.ENTER and ctrl() then add_param(); return end
  if ch==K.ENTER then add_param(); page=1; speak_step_cell(); return end
end
function set_note_in_step(step_index,note)
  local pat=ensure_pattern(current_pattern)
  step_index=clampi(step_index,0,pattern_length()-1)
  local cells=pat.steps[step_index][1]
  -- A newly recorded note replaces a previously recorded Pause on this step.
  for i=#cells,1,-1 do
    if cells[i].id==6 or (cells[i].id==3 and tonumber(cells[i].value)==2) then table.remove(cells,i) end
  end
  local c=find_cell(cells,1)
  if c then c.value=clampi(note,0,127) else table.insert(cells,1,{id=1,value=clampi(note,0,127)}) end
end

function set_pause_in_step(step_index)
  local pat=ensure_pattern(current_pattern)
  step_index=clampi(step_index,0,pattern_length()-1)
  local cells=pat.steps[step_index][1]
  -- A real Pause replaces the Note event on this step.
  for i=#cells,1,-1 do
    if cells[i].id==1 then table.remove(cells,i) end
  end
  local c=find_cell(cells,6)
  if c then c.value=1 else table.insert(cells,1,{id=6,value=1}) end
end

function fill_step_recording_gap(target_step)
  local len=pattern_length()
  local first=clampi(step_recording_step or target_step,0,len)
  target_step=clampi(target_step,0,len-1)
  if first<target_step then
    for st=first,target_step-1 do set_pause_in_step(st) end
  end
end

function finish_or_advance_step_recording(recorded_step,focus_id)
  local pat=ensure_pattern(current_pattern)
  local len=pattern_length()
  current_step=recorded_step
  last_real_step=recorded_step
  current_tab=1
  local cells=pat.steps[recorded_step][1]
  local _,idx=find_cell(cells,focus_id or 1)
  logical_cell=idx or 1
  current_cell=logical_cell
  step_recording_step=recorded_step+1
  if step_recording_step>=len then
    step_recording_active=false
    step_record_sustain_down=false
    set_step_recording_midi_isolation(false)
  else
    current_step=step_recording_step
    last_real_step=current_step
  end
  refresh_pattern_after_edit()
  speak_step_cell()
end

function current_step_record_target()
  if not step_recording_active then return nil end
  -- Step Recording is strictly tied to the real cursor position.
  -- Header, Repeat and Speed rows never record or advance anything.
  if not is_real_step() then return nil end
  return current_step
end

function process_step_record_note(note)
  local target=current_step_record_target()
  if target==nil then return end
  fill_step_recording_gap(target)
  set_note_in_step(target,note)
  finish_or_advance_step_recording(target,1)
end

function process_step_record_pause()
  local target=current_step_record_target()
  if target==nil then return end
  fill_step_recording_gap(target)
  set_pause_in_step(target)
  finish_or_advance_step_recording(target,3)
end

function poll_step_recording_midi()
  if not reaper.MIDI_GetRecentInputEvent then return end
  local newest_seq=reaper.MIDI_GetRecentInputEvent(0)
  if not newest_seq or newest_seq==0 then return end
  if last_recent_midi_seq==nil then last_recent_midi_seq=newest_seq; return end
  if newest_seq==last_recent_midi_seq then return end
  local events={}
  for i=0,63 do
    local seq,buf=reaper.MIDI_GetRecentInputEvent(i)
    if not seq or seq==0 or seq<=last_recent_midi_seq then break end
    events[#events+1]={seq=seq,buf=buf}
  end
  last_recent_midi_seq=newest_seq
  for i=#events,1,-1 do
    local buf=events[i].buf or ""
    local status,d1,d2=buf:byte(1,3)
    if status and d1 and d2 then
      local kind=status & 0xF0
      if kind==0x90 and d2>0 then
        process_step_record_note(d1)
      elseif kind==0xB0 and d1==64 and step_recording_active then
        -- Only the first sustain CC (pedal down) has a musical effect.
        -- Pedal release merely rearms the next press; no timing is measured.
        if d2>=64 then
          if not step_record_sustain_down then
            step_record_sustain_down=true
            process_step_record_pause()
          end
        else
          step_record_sustain_down=false
        end
      end
    end
  end
end


-- Preview scopes: F1/F2 = pattern, F3/F4 = song, F5 = browser.
-- Only one scope may own playback at a time.
function biscot_force_tracker_stop()
  reaper.gmem_attach(GMEM_NS)
  lua_serial=math.floor(reaper.gmem_read(LUA_BASE+1) or lua_serial or 0)+1
  reaper.gmem_write(LUA_BASE+0,LUA_MAGIC)
  reaper.gmem_write(LUA_BASE+2,1)
  reaper.gmem_write(LUA_BASE+3,0)
  reaper.gmem_write(PLAY_STATUS,0)
  reaper.gmem_write(LUA_BASE+1,lua_serial)
  playing=false
  song_playing=false
  song_play_start_time=-1000
  f6_paused=false
end

function biscot_stop_pattern_preview()
  if f5_preview_active then
    f5_preview_active=false
    f5_preview_path=""
  end
  -- Step audition does not always set the Lua playing flag, therefore Stop
  -- must never depend on that flag.
  biscot_force_tracker_stop()
end

function biscot_stop_song_preview()
  -- Song and Song-row auditions can also be active while song_playing is false.
  biscot_force_tracker_stop()
end

function biscot_prepare_preview(target)
  if target=="pattern" then
    biscot_stop_song_preview()
    if f5_preview_active then biscot_stop_f5_preview() end
  elseif target=="song" then
    biscot_stop_pattern_preview()
  elseif target=="browser" then
    biscot_stop_song_preview()
    if playing and not f5_preview_active then
      playing=false
      push_pattern(false)
    end
  end
end

function biscot_page_scope(pg)
  if pg==1 or pg==2 then return "pattern" end
  if pg==3 or pg==4 then return "song" end
  if pg==5 then return "browser" end
  if pg==6 then return "song" end
  return "none"
end

function biscot_change_page(new_page)
  local old_scope=biscot_page_scope(page)
  local new_scope=biscot_page_scope(new_page)
  if old_scope~=new_scope then
    if old_scope=="pattern" then biscot_stop_pattern_preview()
    elseif old_scope=="song" then biscot_stop_song_preview()
    elseif old_scope=="browser" then biscot_stop_f5_preview() end
  end
  page=new_page
  update_midi_trigger_page_mode()
  biscot_set_core_midi_mode_for_page(page)
  publish_song_mixer_state()
  update_controller_title()
end

function draw()
  if overwrite_prompt_active then biscot_draw_overwrite_prompt(); return end
  gfx.set(1,1,1,1); gfx.x=10; gfx.y=10; gfx.drawstr("Biscot1 TrackerControl 0.165 chronological diagnostics")
  gfx.x=10; gfx.y=30; gfx.drawstr("F1 Pattern, F2 Parameters, F3 Song, F4 Song Parameters, F5 Browser, F6 Diagnostics")
  local f6line=(page==6 and (select(2,build_f6_diagnostic_report()))[f6_diag_index]) or nil
  gfx.x=10; gfx.y=50; gfx.drawstr(page==6 and (f6line or "Diagnostics") or page==3 and ((song_lane==4 and "Song global" or ("Song lane "..song_lane.." step "..song_step)).." | Mixer OP"..song_mixer_lane.." "..clampi((ensure_song_mixer(song))[song_mixer_lane] or 100,0,100).."%") or page==4 and ("Song global "..F4_LIST[f4_index]) or is_header_row() and string.format("P%d Header",current_pattern) or is_repeat_row() and string.format("P%d Repeat Row",current_pattern) or is_speed_row() and string.format("P%d Speed Row %d",current_pattern,speed_row_number()) or string.format("P%d step %d %s",current_pattern,current_step,TAB_NAMES[current_tab]))
end
function loop()
  poll_step_recording_midi()
  local preview_status=reaper.gmem_read(PLAY_STATUS)
  if song_delay_prime_pending>0 and song_playing and preview_status>=0.5 then
    prime_song_live_delay_state(song)
    song_delay_prime_pending=song_delay_prime_pending-1
  end
  if song_mixer_prime_pending>0 and song_playing and preview_status>=0.5 then
    -- The Tracker may rebuild the Song header in the first audio blocks. Publish
    -- the complete mixer state again after Play is really active, so the Core
    -- never remains on zeroed or stale faders until a manual +/- movement.
    publish_song_mixer_state()
    song_mixer_prime_pending=song_mixer_prime_pending-1
  end
  if playing and preview_status<0.5 then
    playing=false
    if f5_preview_active then
      f5_preview_active=false; f5_preview_path=""; write_pattern_memory()
    end
  end
  if song_playing and preview_status<0.5 then
    local now=(reaper.time_precise and reaper.time_precise()) or os.clock()
    if now-song_play_start_time>=SONG_PLAY_STATUS_GRACE then
      song_playing=false
    end
  end
  local ch=gfx.getchar()
  if ch<0 then
    biscot_force_tracker_stop()
    save_ram_pattern_snapshot(); save_session_state(); return
  end
  if overwrite_prompt_active then
    biscot_handle_overwrite_prompt(ch)
    save_session_state(); draw(); reaper.defer(loop); return
  end
  if ch==K.ESC then
    biscot_force_tracker_stop()
    save_ram_pattern_snapshot(); save_session_state(); return
  end
  if ch==K.F12 then
    biscot_clear_everything()
  elseif biscot_is_ctrl_shift_c(ch) then
    biscot_copy_loader_command_id()
  elseif biscot_is_ctrl_l(ch) then
    if biscot_open_sws_project_startup_action() then return end
  elseif page==5 then
    -- F5 owns navigation keys before the general page dispatcher.
    if ch==K.F1 then biscot_change_page(1); speak_position()
    elseif ch==K.F2 then rename_f5_current()
    elseif ch==K.F3 then biscot_change_page(3); speak_song_position()
    elseif ch==K.F4 then
      -- F4 is available only from a real F3 Song step.
    elseif ch==K.F5 then refresh_f5_entries(f5_entries[f5_index] and f5_entries[f5_index].name or nil); f5_speak()
    elseif ch==K.F6 then biscot_change_page(6); f6_speak()
    else handle_f5(ch) end
  elseif ch==K.F1 then biscot_change_page(1); speak_position()
  elseif ch==K.F2 then
    if page==1 and is_real_step() then
      biscot_change_page(2)
      local list=PARAM_LISTS[current_tab]
      current_param_index=clampi(last_added_param_index[current_tab] or 1,1,math.max(1,#list))
      local p=list[current_param_index]
      speak("Parameters "..TAB_NAMES[current_tab]..". Grid "..param_grid_height..". "..(p and p.name or "empty"))
    end
  elseif ch==K.F3 then biscot_change_page(3); speak_song_position()
  elseif ch==K.F4 then
    if page==3 and song_step>=0 and song_step<song_lane_length(song_lane) then
      biscot_change_page(4); f4_speak()
    end
  elseif ch==K.F5 then
    biscot_change_page(5)
    refresh_f5_entries(f5_entries[f5_index] and f5_entries[f5_index].name or nil); f5_speak()
  elseif ch==K.F6 then
    biscot_change_page(6); f6_speak()
  elseif page==1 then biscot_prepare_pattern_undo_baseline(); handle_f1(ch)
  elseif page==2 then biscot_prepare_pattern_undo_baseline(); handle_f2(ch)
  elseif page==3 then biscot_prepare_song_undo_baseline(); handle_f3(ch)
  elseif page==4 then biscot_prepare_song_undo_baseline(); handle_f4(ch)
  elseif page==6 then handle_f6(ch) end
  save_session_state()
  draw(); reaper.defer(loop)
end

function reverse_bank_param_id(id)
  id=tonumber(id) or 0
  for _,p in ipairs(PARAMS) do
    if p.map and tonumber(p.map[preview_op]) and id==(1000+tonumber(p.map[preview_op])) then return p.id end
  end
  return id
end

function load_online_pattern_bank(force_scan)
  if not (reaper.gmem_attach and reaper.gmem_read) then return false end
  reaper.gmem_attach(GMEM_NS)
  local serial=math.floor(reaper.gmem_read(BANK_CTRL_BASE+1) or 0)
  local count=math.floor(reaper.gmem_read(BANK_CTRL_BASE+2) or 0)
  local header_ok=(math.floor(reaper.gmem_read(BANK_CTRL_BASE+0) or 0)==BANK_MAGIC)

  -- F1 must be able to preview every Pattern that F3 can already play.
  -- After loading a Composition, the raw Pattern directory in GMEM is the
  -- source of truth; serial/count metadata may be stale or temporarily zero.
  -- Therefore, when requested we scan the whole indexed Pattern bank directly
  -- instead of depending on the controller header alone.
  if not header_ok and not force_scan then return false end

  local loaded=0
  local online={}
  local note_for_pattern={}
  for note=0,127 do
    local ref=math.floor(reaper.gmem_read(MIDI_MAP_BASE+note) or 0)
    if ref>0 then note_for_pattern[ref-1]=note end
  end

  for p=0,MAX_PATTERNS-1 do
    local base=math.floor(reaper.gmem_read(BANK_DIR_BASE+p) or 0)
    if base>=BANK_DATA_BASE and base<BANK_MAX_ADDRESS and math.floor(reaper.gmem_read(base+0) or 0)==BANK_MAGIC then
      local pat={length=clampi(reaper.gmem_read(base+2) or 16,1,MAX_STEPS),repeat_count=clampi(reaper.gmem_read(base+3) or 1,1,1000),repeat_speeds={},repeat_transposes={},midi_note=note_for_pattern[p] or -1,release_arp=clampi(reaper.gmem_read(BANK_RELEASE_ARP_BASE+p) or 0,0,1),steps={}}
      local step_dir_rel=math.floor(reaper.gmem_read(base+4) or 0)
      for i=1,pat.repeat_count do
        pat.repeat_speeds[i]=clamp(reaper.gmem_read(base+4+i) or 1,0.25,16)
        pat.repeat_transposes[i]=clampi(reaper.gmem_read(base+4+pat.repeat_count+i) or 0,-96,96)
      end
      for st=0,MAX_STEPS-1 do
        pat.steps[st]={{},{},{}}
        local rel=math.floor(reaper.gmem_read(base+step_dir_rel+st) or 0)
        if rel>0 then
          local sb=base+rel
          local n=clampi(reaper.gmem_read(sb) or 0,0,MAX_CELLS)
          for i=1,n do
            local id=reverse_bank_param_id(reaper.gmem_read(sb+1+(i-1)*2) or 0)
            local value=reaper.gmem_read(sb+2+(i-1)*2) or 0
            local def=PARAM_BY_ID[id]
            local tab=(def and def.tab) or 1
            table.insert(pat.steps[st][tab],{id=id,value=value})
          end
        end
      end
      online[p]=pat
      ram_pattern_loaded[p]=true
      loaded=loaded+1
    end
  end
  if loaded<=0 and (not force_scan) and serial>0 and count>0 then return false end
  if loaded<=0 then return false end
  patterns=online
  ensure_pattern(current_pattern)
  bank_serial=serial
  return true
end


function load_online_song_bank()
  if not (reaper.gmem_attach and reaper.gmem_read) then return false end
  reaper.gmem_attach(GMEM_NS)
  local online={}
  local first=nil
  local loaded=0
  for sn=0,MAX_SONGS-1 do
    local base=SONG_BANK_BASE+sn*SONG_BANK_STRIDE
    if math.floor(reaper.gmem_read(base+0) or 0)==SONG_MAGIC then
      online[sn]=composition_song_from_gmem(sn)
      first=first or sn
      loaded=loaded+1
    end
  end
  if loaded<=0 then return false end
  songs=online
  current_song=clampi(current_song or 0,0,MAX_SONGS-1)
  if not songs[current_song] then current_song=first or 0 end
  song=songs[current_song] or new_empty_song()
  songs[current_song]=song
  return true
end

function biscot_controller_atexit()
  biscot_force_tracker_stop()
end

function start_controller(bank_was_online)
  if reaper.atexit then reaper.atexit(biscot_controller_atexit) end
  ensure_library_root()
  reaper.gmem_attach(GMEM_NS)
  lua_serial=math.floor(reaper.gmem_read(LUA_BASE+1) or 0)

  -- Reopening the controller must be read-only for an already running Tracker.
  -- The old startup path always loaded the project snapshot and immediately
  -- rewrote the whole Song bank, which erased live Songs/Composition data when
  -- that snapshot was empty or older. Prefer the active GMEM Song bank and do
  -- not write anything back when it already contains valid data.
  local song_bank_was_online=load_online_song_bank()
  if not song_bank_was_online then
    load_song_project()
  end
  load_project_display_names()
  if not song_bank_was_online then
    write_all_songs_memory()
    write_song_memory(false)
  end

  update_midi_trigger_page_mode()
  biscot_set_core_midi_mode_for_page(page)
  load_song_mixer_monitor_state(song)
  -- Always initialize the live mixer slots from the active Song when the
  -- controller opens. This is read-only for the Song bank and prevents stale
  -- zero faders left by an older project session.
  publish_song_mixer_state()
  if not bank_was_online and not song_bank_was_online then
    -- A newly reconstructed Tracker starts from the Core's neutral slider map.
    -- This discards parameter history left in an already existing FX instance.
    reset_core_to_defaults()
  end
  if reaper.MIDI_GetRecentInputEvent then last_recent_midi_seq=select(1,reaper.MIDI_GetRecentInputEvent(0)) end
  if not bank_was_online then
    rebuild_midi_pattern_bank()
    write_pattern_memory()
    save_ram_pattern_snapshot()
  end
  f5_virtual_root=true; f5_scope_root=""; f5_scope_label=""; f5_path=""
  refresh_f5_entries()
  update_controller_title()
  biscot_prepare_pattern_undo_baseline()
  biscot_prepare_song_undo_baseline()
  speak("Biscot1 Tracker. F1. P"..tostring(current_pattern)..". Length, "..pattern_length())
  draw()
  loop()
end

prompt_track=nil
prompt_tracker_fx=-1
prompt_core_fx=-1
prompt_mouse_was_down=false
prompt_choice=2 -- 1 Yes, 2 No. Default is No.

function draw_load_prompt()
  gfx.set(0.12,0.12,0.12,1)
  gfx.rect(0,0,gfx.w,gfx.h,1)
  gfx.set(1,1,1,1)
  gfx.x=24; gfx.y=18
  gfx.drawstr("Load Biscot1 Tracker?")

  if prompt_choice==1 then gfx.set(0.55,0.55,0.55,1) else gfx.set(0.25,0.25,0.25,1) end
  gfx.rect(30,58,120,38,1)
  if prompt_choice==2 then gfx.set(0.55,0.55,0.55,1) else gfx.set(0.25,0.25,0.25,1) end
  gfx.rect(210,58,120,38,1)

  gfx.set(1,1,1,1)
  gfx.x=72; gfx.y=69; gfx.drawstr("Yes")
  gfx.x=254; gfx.y=69; gfx.drawstr("No")
  gfx.update()
end

function biscot_clear_everything()
  biscot_force_tracker_stop()
  clear_tracker_runtime_for_new_instance()

  -- Remove every project/session snapshot so the cleared data cannot return
  -- when the controller or project is reopened. Library files are untouched.
  if reaper.SetProjExtState then
    reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"active_pattern","0")
    reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"active_pattern_file","")
    reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"active_pattern_data","")
    reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"active_song","0")
    reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"song_data","")
    reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"midi_trigger_mode","")
    reaper.SetProjExtState(0,PROJECT_EXT_SECTION,"active_page","")
    for i=0,MAX_PATTERNS-1 do
      reaper.SetProjExtState(0,PROJECT_EXT_SECTION,string.format("pattern_%03d",i),"")
      reaper.SetProjExtState(0,PROJECT_EXT_SECTION,string.format("pattern_name_%03d",i),"")
    end
    for i=0,MAX_SONGS-1 do
      reaper.SetProjExtState(0,PROJECT_EXT_SECTION,string.format("song_%02d",i),"")
      reaper.SetProjExtState(0,PROJECT_EXT_SECTION,string.format("song_name_%02d",i),"")
    end
  end
  if reaper.DeleteExtState then
    for i=0,MAX_PATTERNS-1 do
      reaper.DeleteExtState(EXT_SECTION,session_pattern_key(i),false)
    end
  end

  patterns={}
  songs={}
  pattern_names={}
  song_names={}
  ram_pattern_loaded={}
  current_pattern=0
  current_song=0
  current_step=-1
  current_tab=1
  current_cell=1
  logical_cell=1
  last_real_step=0
  song_lane=1
  song_step=-1
  song_cell=1
  current_param_index=1
  f4_index=1
  pattern_clipboard_mode=nil
  whole_pattern_clipboard=nil
  song_clipboard_mode=nil
  whole_song_clipboard=nil
  whole_song_clipboard_source=nil
  pattern_step_clipboard=nil
  song_step_clipboard=nil
  clear_pattern_step_selection()
  clear_song_step_selection()

  ensure_pattern(0)
  song=new_empty_song()
  songs[0]=song
  load_song_mixer_monitor_state(song)

  -- Write the empty Pattern 0 and Song 0 after clearing all bank directories.
  write_pattern_memory()
  write_song_memory(false)
  rebuild_midi_pattern_bank()
  save_ram_pattern_snapshot(nil,true)
  save_song_project()
  save_session_state()
  update_controller_title()
  speak("Tracker cleared")
end

function clear_tracker_runtime_for_new_instance()
  reaper.gmem_attach(GMEM_NS)
  reaper.gmem_write(PLAY_STATUS,0)
  reaper.gmem_write(LUA_BASE+3,0)
  reaper.gmem_write(BANK_CTRL_BASE+0,0)
  reaper.gmem_write(BANK_CTRL_BASE+1,0)
  reaper.gmem_write(BANK_CTRL_BASE+2,0)
  for p=0,MAX_PATTERNS-1 do
    reaper.gmem_write(BANK_DIR_BASE+p,0)
    reaper.gmem_write(BANK_RELEASE_ARP_BASE+p,0)
  end
  for n=0,127 do reaper.gmem_write(MIDI_MAP_BASE+n,0); reaper.gmem_write(SONG_MIDI_MAP_BASE+n,0) end
  for sn=0,MAX_SONGS-1 do reaper.gmem_write(SONG_BANK_BASE+sn*SONG_BANK_STRIDE,0) end
  reaper.gmem_write(SONG_BASE,0)
end

function close_load_prompt(load_it)
  gfx.quit()
  if not load_it then
    speak("Biscot1 Tracker not loaded")
    return
  end
  if not load_required_fx(prompt_track,prompt_tracker_fx,prompt_core_fx) then
    speak("Required JSFX not available")
    return
  end
  clear_tracker_runtime_for_new_instance()
  start_controller(false)
end

function load_prompt_loop()
  draw_load_prompt()
  local ch=gfx.getchar()
  if ch<0 or ch==27 or ch==string.byte("n") or ch==string.byte("N") then
    close_load_prompt(false)
    return
  end
  if ch==string.byte("y") or ch==string.byte("Y") then
    close_load_prompt(true)
    return
  end
  if ch==30064 or ch==1685026670 or ch==1818584692 or ch==1919379572 then
    prompt_choice=(prompt_choice==1) and 2 or 1
    speak(prompt_choice==1 and "Yes" or "No")
    draw_load_prompt()
  elseif ch==13 then
    close_load_prompt(prompt_choice==1)
    return
  end
  local down=((gfx.mouse_cap or 0)&1)==1
  if down and not prompt_mouse_was_down then
    local x,y=gfx.mouse_x or 0,gfx.mouse_y or 0
    if x>=30 and x<=150 and y>=58 and y<=96 then
      prompt_choice=1
      close_load_prompt(true)
      return
    elseif x>=210 and x<=330 and y>=58 and y<=96 then
      prompt_choice=2
      close_load_prompt(false)
      return
    end
  end
  prompt_mouse_was_down=down
  reaper.defer(load_prompt_loop)
end

function ask_load_required_fx()
  prompt_choice=2
  gfx.init("Load Biscot1 Tracker?",360,120)
  speak("Load Biscot1 Tracker? No")
  draw_load_prompt()
  reaper.defer(load_prompt_loop)
end

load_session_state()
reaper.gmem_attach(GMEM_NS)
prompt_track,prompt_tracker_fx,prompt_core_fx=required_fx_state()
if prompt_track and prompt_tracker_fx>=0 and prompt_core_fx>=0 then
  prompt_tracker_fx,prompt_core_fx=ensure_tracker_before_core(prompt_track)
  controller_bank_was_online=load_online_pattern_bank()
  if not controller_bank_was_online then
    load_all_patterns()
    load_all_project_pattern_snapshots()
    if not load_project_pattern_snapshot() then load_ram_pattern_snapshot(current_pattern) end
    ram_pattern_loaded[current_pattern]=true
  end
  start_controller(controller_bank_was_online)
else
  -- A new instance accepted from the Y/N prompt starts completely empty.
  -- Library, Project and Default are only consulted explicitly from F5.
  controller_bank_was_online=false
  patterns={}; current_pattern=0; current_step=-1; current_cell=1; logical_cell=1
  songs={}; current_song=0; song=new_empty_song(); songs[0]=song
  ensure_pattern(0)
  ask_load_required_fx()
end
