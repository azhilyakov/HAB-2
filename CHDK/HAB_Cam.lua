--[[ High Altitude Balloon Camera
@title Balloon Camera
@param    i Shot Interval
 @default i 5
 @range   i 1 60
@param    d Display
 @default d 0
 @values  d 0 1
--]]


props = require("propcase")
set_console_layout(1, 1, 60, 15)
print_screen(-1)
print("HAB Camera")
print(os.date())
sleep(3000)

local is_record, is_video, mode_number
is_record, is_video, mode_number = get_mode()
mode_number = bitand(mode_number, 0xFF)
if (mode_number ~= 2) then
	print("!!! WRONG MODE "..mode_number.." !!!")
	print("!!! WRONG MODE "..mode_number.." !!!")
	print("!!! WRONG MODE "..mode_number.." !!!")
	print("Set P mode and restart camera")
	play_sound(6)
	sleep(3000)
	post_levent_to_ui('PressPowerButton')
end


if (is_record == false) then
	print("Switching to photo capture")
	set_record(1)
	while (get_mode() == false) do sleep(100) end
	sleep(1000)
end


-- conf.subj_dist_override_koef = SD_OVERRIDE_INFINITY
if (get_config_value(108) ~= 2) then
	print("Enabling distance override")
	set_config_value(108, 2)
	sleep(1000)
end


print("Setting AF-Lock")
set_aflock(1)
set_prop(props.AF_LOCK, 1)
sleep(1000)


print("Setting focus to infinity")
set_focus(60000)
sleep(1000)

local focus_check = get_focus()
if ((focus_check ~= -1) and (focus_check < 1000)) then
	print("!!! FOCUS NOT INFINITY !!!")
	print("!!! FOCUS NOT INFINITY !!!")
	print("!!! FOCUS NOT INFINITY !!!")
	play_sound(6)
	sleep(3000)
	post_levent_to_ui('PressPowerButton')
end


local image_count = 0
print("---")

while(true)
do
	if ((image_count == 3) and (d == 0)) then
		print("Turning off display...")
		sleep(2000)
		set_lcd_display(0)
		sleep(1000)
	end

	image_count = image_count + 1
	local battery_voltage = get_vbatt()
	local temperature = get_temperature(0)
	local disk_kb = get_free_disk_space()
	local disk_mb = disk_kb / 1024

	print("Time: "..os.date())
	print("Battery: "..battery_voltage.." mV")
	print("Temp: "..temperature.." C")
	print("Space: "..disk_mb.." MB")

	if (disk_mb < 10) then
		print("Out of disk space!")
		break
	end

	print("Image: "..image_count)
	press("shoot_half")
	repeat sleep(50) until get_shooting() == true

	local av = get_av96()
	local sv = get_sv96()
	local tv = get_tv96()
	local bv = get_bv96()
	local fc = get_focus()
	print("Exposure: AV="..av.." SV="..sv.." TV="..tv.." BV="..bv)
	print("Focus: "..fc)

	press("shoot_full")
	sleep(500)
	release("shoot_full")

	repeat sleep(50) until get_shooting() == false
	release("shoot_half")

	print("File: "..get_exp_count());
	print("---")

--	if (fc ~= -1) then
--		print("Something wrong with focus lock")
--		print("Resetting camera...")
--		sleep(1000)
--		reboot()
--	end

	sleep(i * 1000)
end

print("Power down")
post_levent_to_ui('PressPowerButton')


