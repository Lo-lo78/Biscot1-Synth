-- @description Biscot1 Tracker Project Startup Loader 0.166 protocol-20261013 controller-parity restore
-- @author Lo-lo + GPT
-- Restores the complete project pattern bank into GMEM without opening the controller.

GMEM_NS="Biscot1TrackerBridge"
local SIDECHAIN_LIVE_BASE=19720
local SIDECHAIN_LIVE_STRIDE=3
local SONG_MIXER_LIVE_BASE=19740 -- +7 is owned by Tracker: 1 only during real Song playback
local SONG_MIXER_MAGIC=20260722
local SONG_MIXER_HEADER_MAGIC=202607220
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
LUA_MAGIC=20261101
LUA_BASE=10000
PLAY_STATUS=LUA_BASE+1500
MIDI_MAP_BASE=LUA_BASE+1600
SONG_MIDI_MAP_BASE=LUA_BASE+1728
REPEAT_TRANSPOSE_BASE=13000
RELEASE_ARP_CURRENT=19500
PATTERN_BASE=9600000
MAX_PATTERNS=1000
MAX_STEPS=1024
MAX_CELLS=128
CELL_SIZE=2
STEP_STRIDE=1+MAX_CELLS*CELL_SIZE
BANK_MAGIC=20260713
BANK_RELEASE_ARP_BASE=96000
BANK_CTRL_BASE=98000
BANK_DIR_BASE=100000
BANK_DATA_BASE=110000
BANK_MAX_ADDRESS=9500000
SONG_MAGIC=20260714
SONG_BASE=9900000
SONG_HEADER_SIZE=3020
SONG_MAX_STEPS=4096
SONG_STEP_STRIDE=12
SONG_START_OFFSET_DIVISIONS=32
SONG_START_PACK_BASE=SONG_START_OFFSET_DIVISIONS+1
SONG_LANE_STRIDE=SONG_MAX_STEPS*SONG_STEP_STRIDE
MAX_SONGS=100
SONG_BANK_BASE=10100000
SONG_BANK_STRIDE=SONG_HEADER_SIZE+3*SONG_LANE_STRIDE
DELAY_LIVE_BASE=19700
DELAY_LIVE_STRIDE=5
PROJECT_EXT_SECTION="Biscot1TrackerProjectState"
CORE_MAGIC=20261013

function clamp(v,a,b) v=tonumber(v) or a; if v<a then return a elseif v>b then return b end return v end
function clampi(v,a,b) return math.floor(clamp(v,a,b)+0.5) end
function join_path(a,b)
  local sep=package.config:sub(1,1); a=tostring(a or ""):gsub("[\\/]+$",""); b=tostring(b or "")
  if a=="" then return b elseif b=="" then return a end; return a..sep..b
end
function norm_path(path) return tostring(path or ""):gsub("\\","/"):gsub("/+","/"):gsub("/$","") end
function parent_dir(path) path=norm_path(path); return path:match("^(.*)/[^/]+$") or "" end
function script_dir()
  local info=debug and debug.getinfo and debug.getinfo(1,"S") or nil
  local source=info and tostring(info.source or "") or ""; if source:sub(1,1)=="@" then source=source:sub(2) end
  source=source:gsub("\\","/"); return (source:match("^(.*[/])") or (reaper.GetResourcePath().."/Scripts/Lo-lo - Scripts/Instruments/Biscot1 TrackerControl/")):gsub("[\\/]+$","")
end
function project_directory()
  if not reaper.EnumProjects then return "" end
  local ok,_,path=pcall(reaper.EnumProjects,-1,""); if not ok then return "" end
  path=norm_path(path or ""); if path=="" then return "" end
  if path:lower():match("%.rpp$") or path:lower():match("%.rpp%-bak$") then return parent_dir(path) end
  return path
end
function default_tracker_root_dir() return join_path(script_dir(),"Biscot1 Tracker") end
function project_tracker_root_dir() local pd=project_directory(); return pd~="" and join_path(pd,"Biscot1 Tracker") or "" end
function read_text_file(path) local f=io.open(path,"rb"); if not f then return nil end; local t=f:read("*a") or ""; f:close(); return t end
function file_exists(path) local f=io.open(path,"rb"); if f then f:close(); return true end; return false end
function active_song_name(root) local t=(read_text_file(join_path(root,"active_song_folder.txt")) or ""):gsub("[\r\n]",""); return t~="" and t or "Song_001" end
function song_root_dir(root) return join_path(root,active_song_name(root)) end
function patterns_dir(root) return join_path(song_root_dir(root),"Patterns") end
function songs_dir(root) return join_path(song_root_dir(root),"Song") end
function active_song_file_path(root) return join_path(song_root_dir(root),"active_song_file.txt") end
function active_pattern_number(root)
  local _,ps=reaper.GetProjExtState(0,PROJECT_EXT_SECTION,"active_pattern"); local p=tonumber(ps)
  if p==nil then p=tonumber((read_text_file(join_path(song_root_dir(root),"active_pattern.txt")) or ""):match("%-?%d+")) end
  return clampi(p or 0,0,999)
end
function parse_pattern_text(txt,forced_p)
  if not txt or txt=="" then return nil,nil end
  local p=forced_p
  if p==nil then p=tonumber(txt:match("pattern=(%-?%d+)")) end
  if p==nil then return nil,nil end
  p=clampi(p,0,999)
  local pat={length=16,repeat_count=1,repeat_speeds={1},repeat_transposes={0},midi_note=-1,release_arp=0,steps={}}
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
  for i=1,pat.repeat_count do if pat.repeat_speeds[i]==nil then pat.repeat_speeds[i]=1 end; if pat.repeat_transposes[i]==nil then pat.repeat_transposes[i]=0 end end
  return pat,p
end
function resolved_id(id) id=tonumber(id) or 0; if id>=2000 and id<=2030 then return 1000+10+(id-2000)*3 end; return id end
function pattern_has_content(pat)
  if not pat then return false end
  if (pat.midi_note or -1)>=0 or (pat.length or 16)~=16 or (pat.repeat_count or 1)~=1 or (pat.release_arp or 0)~=0 then return true end
  local rc=clampi(pat.repeat_count or 1,1,1000)
  for i=1,rc do
    if math.abs((pat.repeat_speeds and pat.repeat_speeds[i] or 1)-1)>0.000000000001 then return true end
    if clampi(pat.repeat_transposes and pat.repeat_transposes[i] or 0,-96,96)~=0 then return true end
  end
  for st=0,MAX_STEPS-1 do for tab=1,3 do if #(pat.steps[st][tab] or {})>0 then return true end end end
  return false
end
function choose_tracker_root()
  local pr=project_tracker_root_dir(); if pr~="" then return pr end; return default_tracker_root_dir()
end
function load_complete_pattern_set(root)
  local all={}; local pd=patterns_dir(root); local i=0
  while true do
    local fn=reaper.EnumerateFiles(pd,i); if not fn then break end
    if tostring(fn):lower():match("%.txt$") then local txt=read_text_file(join_path(pd,fn)); local pat,p=parse_pattern_text(txt,nil); if pat and p then all[p]=pat end end
    i=i+1
  end
  -- Project snapshots are authoritative and preserve every unsaved modification.
  for p=0,MAX_PATTERNS-1 do
    local _,txt=reaper.GetProjExtState(0,PROJECT_EXT_SECTION,string.format("pattern_%03d",p))
    local pat=parse_pattern_text(txt,p); if pat then all[p]=pat end
  end
  -- Backward compatibility with 0.062/0.063 active-only snapshot.
  local _,active_txt=reaper.GetProjExtState(0,PROJECT_EXT_SECTION,"active_pattern_data")
  local ap=active_pattern_number(root); local apat=parse_pattern_text(active_txt,ap); if apat then all[ap]=apat end
  return all
end
function write_full_bank(all,current_pattern)
  reaper.gmem_attach(GMEM_NS)
  for p=0,MAX_PATTERNS-1 do reaper.gmem_write(BANK_DIR_BASE+p,0); reaper.gmem_write(BANK_RELEASE_ARP_BASE+p,0) end
  for n=0,127 do reaper.gmem_write(MIDI_MAP_BASE+n,0) end
  local cursor=BANK_DATA_BASE; local count=0; local note_owner={}
  for p=0,MAX_PATTERNS-1 do
    local pat=all[p]
    if pattern_has_content(pat) then
      local rc=clampi(pat.repeat_count or 1,1,1000); local base=cursor; local step_dir_rel=5+rc*2
      reaper.gmem_write(base,BANK_MAGIC); reaper.gmem_write(base+1,p); reaper.gmem_write(base+2,clampi(pat.length or 16,1,MAX_STEPS)); reaper.gmem_write(base+3,rc); reaper.gmem_write(base+4,step_dir_rel)
      for k=1,rc do reaper.gmem_write(base+4+k,clamp(pat.repeat_speeds[k] or 1,0.25,16)); reaper.gmem_write(base+4+rc+k,clampi(pat.repeat_transposes[k] or 0,-96,96)) end
      cursor=base+step_dir_rel+MAX_STEPS
      for st=0,MAX_STEPS-1 do
        local flat={}; for tab=1,3 do for _,c in ipairs(pat.steps[st][tab] or {}) do if #flat<MAX_CELLS then flat[#flat+1]=c end end end
        reaper.gmem_write(base+step_dir_rel+st,cursor-base); reaper.gmem_write(cursor,#flat); cursor=cursor+1
        for _,c in ipairs(flat) do reaper.gmem_write(cursor,resolved_id(c.id)); reaper.gmem_write(cursor+1,tonumber(c.value) or 0); cursor=cursor+2 end
      end
      if cursor>=BANK_MAX_ADDRESS then break end
      reaper.gmem_write(BANK_DIR_BASE+p,base)
      reaper.gmem_write(BANK_RELEASE_ARP_BASE+p,clampi(pat.release_arp or 0,0,1))
      local note=clampi(pat.midi_note or -1,-1,127); if note>=0 and (note_owner[note]==nil or p==current_pattern) then note_owner[note]=p end
      count=count+1
    end
  end
  for note,p in pairs(note_owner) do reaper.gmem_write(MIDI_MAP_BASE+note,p+1) end
  local serial=math.floor(reaper.gmem_read(BANK_CTRL_BASE+1) or 0)+1
  reaper.gmem_write(BANK_CTRL_BASE,BANK_MAGIC); reaper.gmem_write(BANK_CTRL_BASE+2,count); reaper.gmem_write(BANK_CTRL_BASE+3,cursor); reaper.gmem_write(BANK_CTRL_BASE+1,serial)
end
function pattern_from_gmem(p)
  p=clampi(p or 0,0,MAX_PATTERNS-1)
  local base=math.floor(reaper.gmem_read(BANK_DIR_BASE+p) or 0)
  if base<BANK_DATA_BASE or base>=BANK_MAX_ADDRESS or math.floor(reaper.gmem_read(base) or 0)~=BANK_MAGIC then return nil end
  local pat={length=clampi(reaper.gmem_read(base+2) or 16,1,MAX_STEPS),repeat_count=clampi(reaper.gmem_read(base+3) or 1,1,1000),repeat_speeds={},repeat_transposes={},midi_note=-1,release_arp=clampi(reaper.gmem_read(BANK_RELEASE_ARP_BASE+p) or 0,0,1),steps={}}
  for note=0,127 do if math.floor(reaper.gmem_read(MIDI_MAP_BASE+note) or 0)==p+1 then pat.midi_note=note; break end end
  local step_dir_rel=math.floor(reaper.gmem_read(base+4) or 0)
  if step_dir_rel<5 or step_dir_rel>100000 then return nil end
  for i=1,pat.repeat_count do
    pat.repeat_speeds[i]=clamp(reaper.gmem_read(base+4+i) or 1,0.25,16)
    pat.repeat_transposes[i]=clampi(reaper.gmem_read(base+4+pat.repeat_count+i) or 0,-96,96)
  end
  for st=0,MAX_STEPS-1 do
    pat.steps[st]={{},{},{}}
    local rel=math.floor(reaper.gmem_read(base+step_dir_rel+st) or 0)
    if rel>0 then
      local ptr=base+rel
      local count=clampi(reaper.gmem_read(ptr) or 0,0,MAX_CELLS)
      for ci=0,count-1 do
        -- Composition GMEM already contains resolved core IDs. Keep them as-is;
        -- write_full_bank leaves these IDs untouched, while project TXT
        -- overrides still use logical IDs and are resolved normally.
        local rid=math.floor(reaper.gmem_read(ptr+1+ci*2) or 0)
        local val=reaper.gmem_read(ptr+2+ci*2) or 0
        pat.steps[st][1][#pat.steps[st][1]+1]={id=rid,value=val}
      end
    end
  end
  return pat
end

function load_pattern_set_from_gmem()
  local all={}
  for p=0,MAX_PATTERNS-1 do
    local pat=pattern_from_gmem(p)
    if pat then all[p]=pat end
  end
  return all
end

function overlay_project_pattern_slots_on_restored_composition(root)
  if not reaper.GetProjExtState then return false end
  reaper.gmem_attach(GMEM_NS)
  local all=load_pattern_set_from_gmem()
  local any=false
  for p=0,MAX_PATTERNS-1 do
    local _,txt=reaper.GetProjExtState(0,PROJECT_EXT_SECTION,string.format("pattern_%03d",p))
    local pat=parse_pattern_text(txt,p)
    if pat then all[p]=pat; any=true end
  end
  local active=active_pattern_number(root or choose_tracker_root())
  local _,active_txt=reaper.GetProjExtState(0,PROJECT_EXT_SECTION,"active_pattern_data")
  local active_pat=parse_pattern_text(active_txt,active)
  if active_pat then all[active]=active_pat; any=true end
  if any then
    write_full_bank(all,active)
    write_active_preview(all[active])
  end
  return any
end

function write_active_preview(pat)
  if not pat then return end
  reaper.gmem_attach(GMEM_NS)
  for st=0,MAX_STEPS-1 do
    local flat={}; for tab=1,3 do for _,c in ipairs(pat.steps[st][tab] or {}) do if #flat<MAX_CELLS then flat[#flat+1]=c end end end
    local sb=PATTERN_BASE+st*STEP_STRIDE; reaper.gmem_write(sb,#flat)
    for i=1,MAX_CELLS do local cb=sb+1+(i-1)*CELL_SIZE; local c=flat[i]; reaper.gmem_write(cb,c and resolved_id(c.id) or 0); reaper.gmem_write(cb+1,c and (tonumber(c.value) or 0) or 0) end
  end
  reaper.gmem_write(LUA_BASE,LUA_MAGIC); reaper.gmem_write(LUA_BASE+2,1); reaper.gmem_write(LUA_BASE+3,0); reaper.gmem_write(PLAY_STATUS,0)
  reaper.gmem_write(LUA_BASE+4,pat.length or 16); reaper.gmem_write(LUA_BASE+5,pat.repeat_count or 1); reaper.gmem_write(LUA_BASE+6,1); reaper.gmem_write(LUA_BASE+7,1); reaper.gmem_write(RELEASE_ARP_CURRENT,clampi(pat.release_arp or 0,0,1))
  for i=1,1000 do reaper.gmem_write(LUA_BASE+7+i,i<=(pat.repeat_count or 1) and (pat.repeat_speeds[i] or 1) or 0); reaper.gmem_write(REPEAT_TRANSPOSE_BASE+i-1,i<=(pat.repeat_count or 1) and clampi(pat.repeat_transposes[i] or 0,-96,96) or 0) end
  reaper.gmem_write(LUA_BASE+1,math.floor(reaper.gmem_read(LUA_BASE+1) or 0)+1)
end

function empty_song()
  return {length=16,lane_lengths={16,16,16},midi_note=-1,song_end=-1,chain_loop=0,song_loop_target=-1,lane_repeat={1,1,1},lane_speeds={{1},{1},{1}},filter_owner=1,filter_mask=7,lfo_owner={1,1,1},mixer_volumes={100,100,100},mixer_mute_mask=0,mixer_solo_lane=0,lanes={{},{},{}}}
end

function parse_song_text(txt)
  if not txt or not txt:find("BISCOT1_TRACKER_SONG_V1",1,true) then return nil end
  local song=empty_song()
  local song_end_direct=false
  local pattern_window_v=false
  local exact_pattern_windows_v=false
  txt=tostring(txt or ""):gsub("\r\n","\n"):gsub("\r","\n")
  for line in (txt.."\n"):gmatch("([^\n]*)\n") do
    if line=="song_end_direct=1" then song_end_direct=true
    elseif line:match("^song_name=") then local n=line:sub(11):gsub("^%s+",""):gsub("%s+$",""); song.name=(n~="" and n or nil)
    elseif line=="pattern_window_v=1" then pattern_window_v=true
    elseif line=="exact_pattern_windows_v=1" then exact_pattern_windows_v=true
    elseif line:match("^length=") then
      song.length=clampi(tonumber(line:match("=(%-?%d+)")) or 16,1,SONG_MAX_STEPS)
    elseif line:match("^lane_lengths=") then local a,b,c=line:match("=(%-?%d+),(%-?%d+),(%-?%d+)"); song.lane_lengths={clampi(a or song.length,1,SONG_MAX_STEPS),clampi(b or song.length,1,SONG_MAX_STEPS),clampi(c or song.length,1,SONG_MAX_STEPS)}
    elseif line:match("^midi_note=") then
      song.midi_note=clampi(tonumber(line:match("=(%-?%d+)")) or -1,-1,127)
    elseif line:match("^song_end=") then
      song.song_end=clampi(tonumber(line:match("=(%-?%d+)")) or -1,-1,99)
    elseif line:match("^chain_loop=") then
      song.chain_loop=clampi(tonumber(line:match("=(%-?%d+)")) or 0,0,1)
    elseif line:match("^song_loop_target=") then
      song.song_loop_target=clampi(tonumber(line:match("=(%-?%d+)")) or -1,-1,99)
    elseif line:match("^next_song_loop=") then
      song.chain_loop=clampi(tonumber(line:match("=(%-?%d+)")) or 0,0,1)
    elseif line:match("^lane_repeat=") then
      local a,b,c=line:match("=(%-?%d+),(%-?%d+),(%-?%d+)")
      song.lane_repeat={clampi(a or 1,1,1000),clampi(b or 1,1,1000),clampi(c or 1,1,1000)}
    elseif line:match("^mixer_volumes=") then
      local a,b,c=line:match("=(%-?%d+),(%-?%d+),(%-?%d+)")
      song.mixer_volumes={clampi(a or 100,0,100),clampi(b or 100,0,100),clampi(c or 100,0,100)}
    elseif line:match("^mixer_mute_mask=") then
      song.mixer_mute_mask=clampi(tonumber(line:match("=(%-?%d+)")) or 0,0,7)
    elseif line:match("^mixer_solo_lane=") then
      song.mixer_solo_lane=clampi(tonumber(line:match("=(%-?%d+)")) or 0,0,3)
    elseif line:match("^mode=") then
      local old=clampi(tonumber(line:match("=(%-?%d+)")) or 0,0,16)
      local rc=old>=2 and old or 1
      song.lane_repeat={rc,rc,rc}; song.song_end=-1
    elseif line:match("^lane_mode=") then
      local a,b,c=line:match("=(%-?%d+),(%-?%d+),(%-?%d+)")
      local function oldrc(x) x=clampi(x or 0,0,16); return x>=2 and x or 1 end
      song.lane_repeat={oldrc(a),oldrc(b),oldrc(c)}
    elseif line:match("^lane_speed\t") then
      local la,ri,sp=line:match("^lane_speed\t([^\t]+)\t([^\t]+)\t([^\t]+)")
      la=clampi(la or 1,1,3); ri=clampi(ri or 1,1,1000); song.lane_speeds[la][ri]=clamp(sp or 1,0.25,16)
    elseif line:match("^filter_owner=") then
      song.filter_owner=clampi(tonumber(line:match("=(%-?%d+)")) or 1,1,3)
    elseif line:match("^filter_mask=") then
      song.filter_mask=clampi(tonumber(line:match("=(%-?%d+)")) or 7,0,7)
    elseif line:match("^lfo_owner=") then
      local a,b,c=line:match("=(%-?%d+),(%-?%d+),(%-?%d+)")
      song.lfo_owner={clampi(a or 1,1,3),clampi(b or 1,1,3),clampi(c or 1,1,3)}
    elseif line:match("^lane\t") then
      local la,st,pa,sp,tr,rp,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22=line:match("^lane\t([^\t]+)\t([^\t]+)\t([^\t]+)\t([^\t]+)\t([^\t]+)\t([^\t]+)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)\t?([^\t]*)")
      local ps,pl,le,mask,fo,fm,l1,l2,l3,scs,sca,ds,df,dv,dp,so
      if pattern_window_v then ps,pl,le,mask,fo,fm,l1,l2,l3,scs,sca,ds,df,dv,dp,so=a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22
      else
        -- Ignore the obsolete percentage Song Gate. Converting it to Play Length
        -- was the hidden 64 -> 62 truncation that removed the final two steps.
        ps=0
        local pidx=clampi(pa or -1,-1,999)
        local pbase=pidx>=0 and math.floor((reaper.gmem_read(BANK_DIR_BASE+pidx) or 0)+0.5) or 0
        local plen=(pbase>0 and math.floor((reaper.gmem_read(pbase) or 0)+0.5)==BANK_MAGIC) and clampi(reaper.gmem_read(pbase+2) or 1,1,MAX_STEPS) or 1
        pl=plen
        le,mask,fo,fm,l1,l2,l3,scs,sca,ds,df,dv,dp,so=a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,0
      end
      la=clampi(la or 1,1,3); st=clampi(st or 0,0,SONG_MAX_STEPS-1)
      song.lanes[la][st]={pattern=clampi(pa or -1,-1,999),speed=clamp(sp or 1,0.25,16),transpose=clampi(tr or 0,-96,96),repeat_count=clampi(rp or 1,1,16),pattern_start=clampi(ps or 0,0,MAX_STEPS-1),play_length=clampi(pl or 0,0,MAX_STEPS),legato=clampi(tonumber(le) or 0,0,1),filter_owner=clampi(fo or 1,1,3),filter_mask=clampi(fm or 7,0,7),lfo1_owner=clampi(l1 or 1,1,3),lfo2_owner=clampi(l2 or 1,1,3),lfo3_owner=clampi(l3 or 1,1,3),sidechain_source=clampi(scs or 0,0,3),sidechain_amount=clampi(sca or 0,0,100),delay_speed=clamp(ds or 0,0,8),delay_feedback=clampi(df or 0,0,100),delay_volume=clampi(dv or 100,0,100),delay_pingpong_depth=clampi(dp or 0,0,100),start_offset=clampi(so or 0,0,31),present_mask=tonumber(mask) or 63}
    end
  end
  -- Repair files already migrated by an older controller. The new marker
  -- protects intentional Pattern windows, including an intentional Length-2.
  if not exact_pattern_windows_v then
    for lane=1,3 do
      for _,t in pairs(song.lanes[lane] or {}) do
        local pidx=clampi((t and t.pattern) or -1,-1,999)
        if pidx>=0 then
          local pbase=math.floor((reaper.gmem_read(BANK_DIR_BASE+pidx) or 0)+0.5)
          local plen=(pbase>0 and math.floor((reaper.gmem_read(pbase) or 0)+0.5)==BANK_MAGIC) and clampi(reaper.gmem_read(pbase+2) or 1,1,MAX_STEPS) or 1
          local pst=clampi(t.pattern_start or 0,0,plen-1)
          local pl=clampi(t.play_length or 0,0,MAX_STEPS)
          if pst==0 and plen>=3 and pl==plen-2 then t.play_length=plen
          elseif pl<=0 then t.play_length=math.max(1,plen-pst) end
        end
      end
    end
  end
  song.lane_speeds=song.lane_speeds or {{1},{1},{1}}
  for lane=1,3 do
    song.lane_repeat[lane]=clampi(song.lane_repeat[lane] or 1,1,1000)
    local last=1
    for ri=1,1000 do if song.lane_speeds[lane][ri]==nil then song.lane_speeds[lane][ri]=last else last=song.lane_speeds[lane][ri] end end
  end
  if not song_end_direct then song.song_end=clampi((song.song_end or 0)-1,-1,99) end
  return song
end

function load_song_set(root)
  local songs={}
  local loaded=false
  for i=0,MAX_SONGS-1 do
    local _,txt=reaper.GetProjExtState(0,PROJECT_EXT_SECTION,string.format("song_%02d",i))
    local song=parse_song_text(txt)
    if song then songs[i]=song; loaded=true end
  end

  local _,active_txt=reaper.GetProjExtState(0,PROJECT_EXT_SECTION,"active_song")
  local active=clampi(tonumber(active_txt) or 0,0,MAX_SONGS-1)

  if not loaded then
    local _,legacy=reaper.GetProjExtState(0,PROJECT_EXT_SECTION,"song_data")
    local song=parse_song_text(legacy)
    if song then songs[active]=song; loaded=true end
  end

  if not loaded then
    local sf=(read_text_file(active_song_file_path(root)) or ""):gsub("[\r\n]","")
    if sf~="" then
      local song=parse_song_text(read_text_file(sf) or "")
      if song then songs[active]=song; loaded=true end
    end
  end

  if not songs[active] then songs[active]=empty_song() end
  if reaper.SetProjExtState then
    for i=0,MAX_SONGS-1 do
      local n=songs[i] and songs[i].name or ""
      if n~="" then reaper.SetProjExtState(0,PROJECT_EXT_SECTION,string.format("song_name_%02d",i),n) end
    end
  end
  return songs,active
end

function write_one_song(song,base,song_index)
  song=song or empty_song()
  reaper.gmem_write(base+0,SONG_MAGIC)
  song.lane_lengths=song.lane_lengths or {song.length or 16,song.length or 16,song.length or 16}
  song.length=math.max(song.lane_lengths[1],song.lane_lengths[2],song.lane_lengths[3])
  reaper.gmem_write(base+1,clampi(song.length or 16,1,SONG_MAX_STEPS))
  for lane=1,3 do reaper.gmem_write(base+3013+lane,clampi(song.lane_lengths[lane] or song.length,1,SONG_MAX_STEPS)) end
  reaper.gmem_write(base+3017,clampi(song.midi_note or -1,-1,127))
  reaper.gmem_write(base+2,clampi(song.song_end or -1,-1,99))
  reaper.gmem_write(base+3011,0)
  reaper.gmem_write(base+3012,-1)
  reaper.gmem_write(base+3013,clampi(song_index or 0,0,99))
  song.mixer_volumes=song.mixer_volumes or {100,100,100}
  local v1=clampi(song.mixer_volumes[1] or 100,0,100)
  local v2=clampi(song.mixer_volumes[2] or 100,0,100)
  local v3=clampi(song.mixer_volumes[3] or 100,0,100)
  local volumes_packed=1+v1+v2*101+v3*10201
  local state_packed=SONG_MIXER_HEADER_MAGIC+clampi(song.mixer_mute_mask or 0,0,7)+clampi(song.mixer_solo_lane or 0,0,3)*8
  reaper.gmem_write(base+3018,volumes_packed)
  reaper.gmem_write(base+3019,state_packed)
  reaper.gmem_write(base+3,clampi(song.filter_owner or 1,1,3))
  reaper.gmem_write(base+4,clampi(song.filter_mask or 7,0,7))
  for i=1,3 do reaper.gmem_write(base+4+i,clampi((song.lfo_owner or {})[i] or 1,1,3)) end
  song.lane_repeat=song.lane_repeat or {1,1,1}; song.lane_speeds=song.lane_speeds or {{1},{1},{1}}
  for lane=1,3 do
    reaper.gmem_write(base+7+lane,clampi(song.lane_repeat[lane] or 1,1,1000))
    local last=1
    for ri=1,1000 do local sp=song.lane_speeds[lane][ri]; if sp==nil then sp=last else last=sp end; reaper.gmem_write(base+10+(lane-1)*1000+ri,clamp(sp,0.25,16)) end
  end
  for lane=1,3 do
    local lb=base+SONG_HEADER_SIZE+(lane-1)*SONG_LANE_STRIDE
    for st=0,SONG_MAX_STEPS-1 do
      local t=(song.lanes and song.lanes[lane]) and song.lanes[lane][st] or nil
      local b=lb+st*SONG_STEP_STRIDE
      local pm=t and (t.present_mask or 63) or 0
      local function hasbit(i) return math.floor(pm/(2^(i-1)))%2==1 end
      local fo=t and hasbit(10) and clampi(t.filter_owner or 1,1,3) or 0
      local fm=t and hasbit(11) and (clampi(t.filter_mask or 7,0,7)+1) or 0
      local l1=t and hasbit(12) and clampi(t.lfo1_owner or 1,1,3) or 0
      local l2=t and hasbit(13) and clampi(t.lfo2_owner or 1,1,3) or 0
      local l3=t and hasbit(14) and clampi(t.lfo3_owner or 1,1,3) or 0
      local scs=t and hasbit(15) and (clampi(t.sidechain_source or 0,0,3)+1) or 0
      local sca=t and hasbit(16) and (clampi(t.sidechain_amount or 0,0,100)+1) or 0
      reaper.gmem_write(b,(t and ((t.pattern or -1)+1) or 0)+fo*2048)
      reaper.gmem_write(b+1,(t and (t.speed or 1) or 1)+fm*32)
      reaper.gmem_write(b+2,clampi((t and (t.transpose or 0) or 0),-96,96)+128+l1*256)
      reaper.gmem_write(b+3,(t and (t.repeat_count or 1) or 1)+l2*32)
      reaper.gmem_write(b+4,(t and (t.legato or 0) or 0)+l3*2)
      reaper.gmem_write(b+5,scs*2+sca*10)
      local ds=t and hasbit(17) and (math.floor(clamp(t.delay_speed or 0,0,8)*8+0.5)+1) or 0
      local df=t and hasbit(18) and (clampi(t.delay_feedback or 0,0,100)+1) or 0
      local dv=t and hasbit(19) and (clampi(t.delay_volume or 100,0,100)+1) or 0
      local dp=t and hasbit(20) and (clampi(t.delay_pingpong_depth or 0,0,100)+1) or 0
      reaper.gmem_write(b+6,ds)
      reaper.gmem_write(b+7,df)
      reaper.gmem_write(b+8,dv)
      reaper.gmem_write(b+9,dp)
      local pat=(t and clampi(t.pattern or -1,-1,999)) or -1
      local pbase=pat>=0 and math.floor((reaper.gmem_read(BANK_DIR_BASE+pat) or 0)+0.5) or 0
      local plen=(pbase>0 and math.floor((reaper.gmem_read(pbase) or 0)+0.5)==BANK_MAGIC) and clampi(reaper.gmem_read(pbase+2) or 1,1,MAX_STEPS) or 1
      local pst=t and clampi(t.pattern_start or 0,0,math.max(0,plen-1)) or 0
      local raw_pl=t and tonumber(t.play_length or 0) or 0
      local pl=(raw_pl and raw_pl>0) and clampi(raw_pl,1,math.max(1,plen-pst)) or math.max(1,plen-pst)
      local start_offset_code=t and hasbit(22) and (clampi(t.start_offset or 0,0,31)+1) or 0
      reaper.gmem_write(b+10,pst*SONG_START_PACK_BASE+start_offset_code)
      reaper.gmem_write(b+11,t and pl or 0)
    end
  end
end

function prime_delay_live_from_song_memory(base)
  -- Startup must publish the same latched delay state as the controller.
  -- Scan FX-only rows too, stopping after the first playable Pattern row.
  reaper.gmem_attach(GMEM_NS)
  for lane=1,3 do
    local live=DELAY_LIVE_BASE+(lane-1)*DELAY_LIVE_STRIDE
    local speed=clamp(reaper.gmem_read(live+0) or 0,0,8)
    local feedback=clampi(reaper.gmem_read(live+1) or 0,0,100)
    local volume=clampi(reaper.gmem_read(live+2) or 100,0,100)
    local ping=clampi(reaper.gmem_read(live+3) or 0,0,100)
    local changed=false
    local len=clampi(reaper.gmem_read(base+3013+lane) or reaper.gmem_read(base+1) or 16,1,SONG_MAX_STEPS)
    local lb=base+SONG_HEADER_SIZE+(lane-1)*SONG_LANE_STRIDE
    for st=0,len-1 do
      local b=lb+st*SONG_STEP_STRIDE
      local ds=math.max(0,math.floor((reaper.gmem_read(b+6) or 0)+0.5))
      local df=math.max(0,math.floor((reaper.gmem_read(b+7) or 0)+0.5))
      local dv=math.max(0,math.floor((reaper.gmem_read(b+8) or 0)+0.5))
      local dp=math.max(0,math.floor((reaper.gmem_read(b+9) or 0)+0.5))
      if ds>0 then speed=clamp((ds-1)/8,0,8); changed=true end
      if df>0 then feedback=clampi(df-1,0,100); changed=true end
      if dv>0 then volume=clampi(dv-1,0,100); changed=true end
      if dp>0 then ping=clampi(dp-1,0,100); changed=true end
      local packed=math.max(0,reaper.gmem_read(b) or 0)
      if math.floor(packed%2048)>0 then break end
    end
    if changed then
      reaper.gmem_write(live+0,speed)
      reaper.gmem_write(live+1,feedback)
      reaper.gmem_write(live+2,volume)
      reaper.gmem_write(live+3,ping)
      reaper.gmem_write(live+4,math.floor(reaper.gmem_read(live+4) or 0)+1)
    end
  end
end

function prime_song_mixer_live_from_song_memory(base)
  reaper.gmem_attach(GMEM_NS)
  local vol_raw=math.floor((reaper.gmem_read(base+3018) or 0)+0.5)
  local state_raw=math.floor((reaper.gmem_read(base+3019) or 0)+0.5)
  local state=state_raw-SONG_MIXER_HEADER_MAGIC
  local v1,v2,v3,mute_mask,solo_lane=100,100,100,0,0
  if state>=0 and state<=31 and vol_raw>0 then
    local packed=math.max(0,vol_raw-1)
    v1=packed%101; packed=math.floor(packed/101)
    v2=packed%101; v3=math.floor(packed/101)%101
    mute_mask=state%8; solo_lane=math.floor(state/8)%4
  end
  reaper.gmem_write(SONG_MIXER_LIVE_BASE+0,clampi(v1,0,100))
  reaper.gmem_write(SONG_MIXER_LIVE_BASE+1,clampi(v2,0,100))
  reaper.gmem_write(SONG_MIXER_LIVE_BASE+2,clampi(v3,0,100))
  reaper.gmem_write(SONG_MIXER_LIVE_BASE+3,clampi(mute_mask,0,7))
  reaper.gmem_write(SONG_MIXER_LIVE_BASE+4,clampi(solo_lane,0,3))
  reaper.gmem_write(SONG_MIXER_LIVE_BASE+6,SONG_MIXER_MAGIC)
  -- Restore values only. The Tracker raises +7 when Song playback actually starts.
  reaper.gmem_write(SONG_MIXER_LIVE_BASE+7,0)
  reaper.gmem_write(SONG_MIXER_LIVE_BASE+5,math.floor(reaper.gmem_read(SONG_MIXER_LIVE_BASE+5) or 0)+1)
end

function rebuild_song_midi_map_from_bank()
  reaper.gmem_attach(GMEM_NS)
  for note=0,127 do reaper.gmem_write(SONG_MIDI_MAP_BASE+note,0) end
  local owner={}
  for sn=0,MAX_SONGS-1 do
    local base=SONG_BANK_BASE+sn*SONG_BANK_STRIDE
    if math.floor((reaper.gmem_read(base) or 0)+0.5)==SONG_MAGIC then
      local note=clampi(reaper.gmem_read(base+3017) or -1,-1,127)
      if note>=0 then owner[note]=sn end
    end
  end
  for note,sn in pairs(owner) do reaper.gmem_write(SONG_MIDI_MAP_BASE+note,sn+1) end
end

function write_complete_song_bank(songs,active)
  reaper.gmem_attach(GMEM_NS)
  for i=0,MAX_SONGS-1 do
    if songs[i] then
      write_one_song(songs[i],SONG_BANK_BASE+i*SONG_BANK_STRIDE,i)
    else
      reaper.gmem_write(SONG_BANK_BASE+i*SONG_BANK_STRIDE,0)
    end
  end
  write_one_song(songs[active],SONG_BASE,active)
  rebuild_song_midi_map_from_bank()
  prime_delay_live_from_song_memory(SONG_BASE)
  prime_song_mixer_live_from_song_memory(SONG_BASE)
end


function composition_apply_run(start_addr,rel,count,value)
  rel=math.max(0,math.floor(tonumber(rel) or 0))
  count=math.max(0,math.floor(tonumber(count) or 0))
  value=tonumber(value) or 0
  for i=0,count-1 do reaper.gmem_write(start_addr+rel+i,value) end
end

function copy_gmem_range(src,dst,count)
  for i=0,count-1 do reaper.gmem_write(dst+i,reaper.gmem_read(src+i) or 0) end
end

function restore_raw_composition(txt)
  if not txt or not txt:find("BISCOT1_TRACKER_COMPOSITION_V8_RAW_GMEM",1,true) then return false end
  reaper.gmem_attach(GMEM_NS)

  local old_cursor=math.floor(reaper.gmem_read(BANK_CTRL_BASE+3) or BANK_DATA_BASE)
  local new_size=math.max(0,tonumber(txt:match("pattern_data_size=(%d+)")) or 0)
  local clear_size=math.min(math.max(0,math.max(old_cursor-BANK_DATA_BASE,new_size)),BANK_MAX_ADDRESS-BANK_DATA_BASE)
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
    if sn then song_lengths[clampi(sn,0,MAX_SONGS-1)]=clampi(len,1,SONG_MAX_STEPS) end
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
    if sn then composition_apply_run(SONG_BANK_BASE+clampi(sn,0,MAX_SONGS-1)*SONG_BANK_STRIDE,r,c,v) end
    local sx,lane,rr,cc,vv=line:match("^song_lane\t(%d+),(%d+)\t(%d+)\t(%d+)\t([^\t]+)$")
    if sx then
      local base=SONG_BANK_BASE+clampi(sx,0,MAX_SONGS-1)*SONG_BANK_STRIDE+SONG_HEADER_SIZE+(clampi(lane,1,3)-1)*SONG_LANE_STRIDE
      composition_apply_run(base,rr,cc,vv)
    end
  end

  local active_song=clampi(tonumber(txt:match("active_song=(%-?%d+)")) or 0,0,MAX_SONGS-1)
  local src=SONG_BANK_BASE+active_song*SONG_BANK_STRIDE
  if math.floor(reaper.gmem_read(src) or 0)==SONG_MAGIC then
    copy_gmem_range(src,SONG_BASE,SONG_HEADER_SIZE)
    local len=clampi(reaper.gmem_read(src+1) or 16,1,SONG_MAX_STEPS)
    local cells=len*SONG_STEP_STRIDE
    for lane=1,3 do
      copy_gmem_range(src+SONG_HEADER_SIZE+(lane-1)*SONG_LANE_STRIDE,SONG_BASE+SONG_HEADER_SIZE+(lane-1)*SONG_LANE_STRIDE,cells)
    end
    prime_delay_live_from_song_memory(SONG_BASE)
    prime_song_mixer_live_from_song_memory(SONG_BASE)
  end
  rebuild_song_midi_map_from_bank()
  return true
end

-- A saved raw Composition is the broad project snapshot, but the controller may
-- later edit and save one Song (especially its F3 mixer) without rebuilding the
-- whole Composition.  Overlay the newer per-Song project states after restoring
-- composition_raw, so the latest Song and mixer always win.
function overlay_project_song_slots_on_restored_composition()
  if not reaper.GetProjExtState then return false end
  reaper.gmem_attach(GMEM_NS)
  local any=false
  local active_txt=""
  local _,active_state=reaper.GetProjExtState(0,PROJECT_EXT_SECTION,"active_song")
  local active=clampi(tonumber(active_state) or 0,0,MAX_SONGS-1)

  for i=0,MAX_SONGS-1 do
    local _,txt=reaper.GetProjExtState(0,PROJECT_EXT_SECTION,string.format("song_%02d",i))
    local song=parse_song_text(txt)
    if song then
      write_one_song(song,SONG_BANK_BASE+i*SONG_BANK_STRIDE,i)
      any=true
      if i==active then active_txt=txt end
    end
  end

  -- Backward-compatible active Song snapshot when no indexed active slot exists.
  if active_txt=="" then
    local _,legacy=reaper.GetProjExtState(0,PROJECT_EXT_SECTION,"song_data")
    local legacy_song=parse_song_text(legacy)
    if legacy_song then
      write_one_song(legacy_song,SONG_BANK_BASE+active*SONG_BANK_STRIDE,active)
      any=true
    end
  end

  local src=SONG_BANK_BASE+active*SONG_BANK_STRIDE
  if math.floor(reaper.gmem_read(src) or 0)==SONG_MAGIC then
    copy_gmem_range(src,SONG_BASE,SONG_HEADER_SIZE)
    for lane=1,3 do
      local len=clampi(reaper.gmem_read(src+3013+lane) or reaper.gmem_read(src+1) or 16,1,SONG_MAX_STEPS)
      copy_gmem_range(src+SONG_HEADER_SIZE+(lane-1)*SONG_LANE_STRIDE,
                      SONG_BASE+SONG_HEADER_SIZE+(lane-1)*SONG_LANE_STRIDE,
                      len*SONG_STEP_STRIDE)
    end
    prime_delay_live_from_song_memory(SONG_BASE)
    prime_song_mixer_live_from_song_memory(SONG_BASE)
  end
  rebuild_song_midi_map_from_bank()
  return any
end

function reorder_tracker_before_core(tr,tracker_fx,core_fx)
  if not tr or tracker_fx<0 or core_fx<0 then return tracker_fx,core_fx end
  if tracker_fx>core_fx and reaper.TrackFX_CopyToTrack then
    reaper.TrackFX_CopyToTrack(tr,tracker_fx,tr,core_fx,true)
    tracker_fx,core_fx=-1,-1
    for fx=0,reaper.TrackFX_GetCount(tr)-1 do
      local ok,name=reaper.TrackFX_GetFXName(tr,fx,"")
      local n=tostring(ok and name or ""):lower()
      if n:find("biscot1",1,true) and n:find("tracker",1,true) then tracker_fx=fx
      elseif n:find("biscot1",1,true) then core_fx=fx end
    end
  end
  return tracker_fx,core_fx
end

function find_tracker_core_pairs()
  local out={}
  for ti=0,reaper.CountTracks(0)-1 do
    local tr=reaper.GetTrack(0,ti)
    local tracker_fx,core_fx=-1,-1
    for fx=0,reaper.TrackFX_GetCount(tr)-1 do
      local ok,name=reaper.TrackFX_GetFXName(tr,fx,"")
      local n=tostring(ok and name or ""):lower()
      if n:find("biscot1",1,true) and n:find("tracker",1,true) then tracker_fx=fx
      elseif n:find("biscot1",1,true) then core_fx=fx end
    end
    if tracker_fx>=0 and core_fx>=0 then
      tracker_fx,core_fx=reorder_tracker_before_core(tr,tracker_fx,core_fx)
      if tracker_fx>=0 and core_fx>=0 then
        out[#out+1]={track=tr,tracker=tracker_fx,core=core_fx}
      end
    end
  end
  return out
end

function bridge_is_ready()
  reaper.gmem_attach(GMEM_NS)
  return math.floor((reaper.gmem_read(0) or 0)+0.5)==CORE_MAGIC and (reaper.gmem_read(3) or 0)>0
end

function enforce_runtime_fx_state(fx_pairs,initialize_core)
  for _,pair in ipairs(fx_pairs or {}) do
    if initialize_core then
      -- Project chunks may contain the final slider state of the previous
      -- playback. Rebuild the Tracker from a deterministic neutral Core.
      for slider,value in pairs(CORE_SLIDER_DEFAULTS) do
        reaper.TrackFX_SetParam(pair.track,pair.core,slider-1,value)
      end
    end
    -- Permanent Tracker infrastructure: MIDI channels 1-3 and Tracker enabled.
    if reaper.TrackFX_SetParamNormalized then
      reaper.TrackFX_SetParamNormalized(pair.track,pair.core,2,1.0)
      reaper.TrackFX_SetParamNormalized(pair.track,pair.tracker,0,1.0)
    else
      reaper.TrackFX_SetParam(pair.track,pair.core,2,2.0)
      reaper.TrackFX_SetParam(pair.track,pair.tracker,0,1.0)
    end
  end
end

function repair_legacy_two_step_windows_in_gmem()
  local _,exact_state=reaper.GetProjExtState(0,PROJECT_EXT_SECTION,"pattern_window_exact_v")
  if tostring(exact_state or "")=="1" then return false end
  local changed=false
  for sn=0,MAX_SONGS-1 do
    local base=SONG_BANK_BASE+sn*SONG_BANK_STRIDE
    if math.floor((reaper.gmem_read(base) or 0)+0.5)==SONG_MAGIC then
      for lane=1,3 do
        local len=clampi(reaper.gmem_read(base+3013+lane) or reaper.gmem_read(base+1) or 16,1,SONG_MAX_STEPS)
        local lb=base+SONG_HEADER_SIZE+(lane-1)*SONG_LANE_STRIDE
        for st=0,len-1 do
          local b=lb+st*SONG_STEP_STRIDE
          local packed_ref=math.max(0,math.floor((reaper.gmem_read(b) or 0)+0.5))
          local ref=packed_ref%2048
          if ref>0 then
            local pidx=ref-1
            local pbase=math.floor((reaper.gmem_read(BANK_DIR_BASE+pidx) or 0)+0.5)
            if pbase>0 and math.floor((reaper.gmem_read(pbase) or 0)+0.5)==BANK_MAGIC then
              local plen=clampi(reaper.gmem_read(pbase+2) or 1,1,MAX_STEPS)
              local packed_start=math.max(0,math.floor((reaper.gmem_read(b+10) or 0)+0.5))
              local pst=clampi(math.floor(packed_start/SONG_START_PACK_BASE),0,plen-1)
              local pl=clampi(reaper.gmem_read(b+11) or 0,0,MAX_STEPS)
              if pst==0 and plen>=3 and pl==plen-2 then
                reaper.gmem_write(b+11,plen)
                changed=true
              end
            end
          end
        end
      end
    end
  end
  return changed
end

function activate_song_slot(active_song)
  reaper.gmem_attach(GMEM_NS)
  active_song=clampi(active_song or 0,0,MAX_SONGS-1)
  local src=SONG_BANK_BASE+active_song*SONG_BANK_STRIDE
  if math.floor((reaper.gmem_read(src) or 0)+0.5)~=SONG_MAGIC then return false end
  copy_gmem_range(src,SONG_BASE,SONG_HEADER_SIZE)
  for lane=1,3 do
    local len=clampi(reaper.gmem_read(src+3013+lane) or reaper.gmem_read(src+1) or 16,1,SONG_MAX_STEPS)
    copy_gmem_range(src+SONG_HEADER_SIZE+(lane-1)*SONG_LANE_STRIDE,
                    SONG_BASE+SONG_HEADER_SIZE+(lane-1)*SONG_LANE_STRIDE,
                    len*SONG_STEP_STRIDE)
  end
  prime_delay_live_from_song_memory(SONG_BASE)
  prime_song_mixer_live_from_song_memory(SONG_BASE)
  return true
end

function validate_pattern_bank_for_song(active_song)
  reaper.gmem_attach(GMEM_NS)
  if math.floor((reaper.gmem_read(BANK_CTRL_BASE) or 0)+0.5)~=BANK_MAGIC then return false end
  local sb=SONG_BANK_BASE+clampi(active_song or 0,0,MAX_SONGS-1)*SONG_BANK_STRIDE
  if math.floor((reaper.gmem_read(sb) or 0)+0.5)~=SONG_MAGIC then return false end
  for lane=1,3 do
    local len=clampi(reaper.gmem_read(sb+3013+lane) or reaper.gmem_read(sb+1) or 16,1,SONG_MAX_STEPS)
    local lb=sb+SONG_HEADER_SIZE+(lane-1)*SONG_LANE_STRIDE
    for st=0,len-1 do
      local packed=math.max(0,math.floor((reaper.gmem_read(lb+st*SONG_STEP_STRIDE) or 0)+0.5))
      local pat=(packed%2048)-1
      if pat>=0 then
        local pb=math.floor((reaper.gmem_read(BANK_DIR_BASE+pat) or 0)+0.5)
        if pb<BANK_DATA_BASE or pb>=BANK_MAX_ADDRESS or math.floor((reaper.gmem_read(pb) or 0)+0.5)~=BANK_MAGIC then return false end
      end
    end
  end
  return true
end

local function clear_live_sidechain_state()
  reaper.gmem_attach(GMEM_NS)
  for lane=1,3 do
    local b=SIDECHAIN_LIVE_BASE+(lane-1)*SIDECHAIN_LIVE_STRIDE
    reaper.gmem_write(b+0,0)
    reaper.gmem_write(b+1,0)
    reaper.gmem_write(b+2,math.floor(reaper.gmem_read(b+2) or 0)+1)
  end
end

function publish_stop_handshake()
  reaper.gmem_attach(GMEM_NS)
  reaper.gmem_write(PLAY_STATUS,0)
  reaper.gmem_write(SONG_MIXER_LIVE_BASE+7,0)
  reaper.gmem_write(LUA_BASE,LUA_MAGIC)
  reaper.gmem_write(LUA_BASE+2,1)
  reaper.gmem_write(LUA_BASE+3,0)
  reaper.gmem_write(LUA_BASE+1,math.floor(reaper.gmem_read(LUA_BASE+1) or 0)+1)
end

startup_try=0
startup_done=false
startup_pairs={}

function perform_exact_restore()
  local root=choose_tracker_root()
  local active_pattern=active_pattern_number(root)
  local _,active_song_state=reaper.GetProjExtState(0,PROJECT_EXT_SECTION,"active_song")
  local active_song=clampi(tonumber(active_song_state) or 0,0,MAX_SONGS-1)

  publish_stop_handshake()

  local _,raw=reaper.GetProjExtState(0,PROJECT_EXT_SECTION,"composition_raw")
  local restored=restore_raw_composition(raw)
  if restored then
    local _,pattern_override=reaper.GetProjExtState(0,PROJECT_EXT_SECTION,"pattern_slots_override_composition")
    if tostring(pattern_override or "")=="1" then overlay_project_pattern_slots_on_restored_composition(root) end
    local _,song_override=reaper.GetProjExtState(0,PROJECT_EXT_SECTION,"song_slots_override_composition")
    if tostring(song_override or "")=="1" then overlay_project_song_slots_on_restored_composition() end
  end

  -- A raw snapshot is accepted only if the active Song can resolve every Pattern
  -- it references. Otherwise rebuild from the controller-compatible project states.
  if not restored or not validate_pattern_bank_for_song(active_song) then
    local all=load_complete_pattern_set(root)
    local songs
    songs,active_song=load_song_set(root)
    write_full_bank(all,active_pattern)
    write_complete_song_bank(songs,active_song)
  else
    repair_legacy_two_step_windows_in_gmem()
    activate_song_slot(active_song)
    rebuild_song_midi_map_from_bank()
  end

  clear_live_sidechain_state()
  local active_pat=pattern_from_gmem(active_pattern)
  if active_pat then write_active_preview(active_pat) else publish_stop_handshake() end

  -- The controller performs this initialization whenever no valid online bank
  -- existed and it had to rebuild the project. A project startup is the same
  -- condition: GMEM has just been reconstructed from the RPP/Composition.
  enforce_runtime_fx_state(startup_pairs,true)
  startup_done=true
end

function startup_loop()
  startup_try=startup_try+1
  startup_pairs=find_tracker_core_pairs()
  local ready=#startup_pairs>0 and bridge_is_ready()
  if not ready and startup_try<120 then
    reaper.defer(startup_loop)
    return
  end

  if not startup_done then
    perform_exact_restore()
  end

  -- REAPER can restore FX parameter chunks slightly after the project startup
  -- action. Reassert only the two permanent infrastructure switches. Operator
  -- levels are initialized once, exactly as start_controller(false) does, so
  -- this loop cannot overwrite musical Level events after playback starts.
  enforce_runtime_fx_state(startup_pairs,false)
  if startup_try<126 then reaper.defer(startup_loop) end
end

reaper.defer(startup_loop)
