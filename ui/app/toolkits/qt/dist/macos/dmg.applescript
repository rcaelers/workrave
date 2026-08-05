on run -- for testing in script editor
	tell application "Finder"
		tell disk "Workrave"
			open
			tell container window
				set current view to icon view
				set toolbar visible to false
				set statusbar visible to false
				set the bounds to {100, 100, 780, 600}
			end tell
			close
			set opts to the icon view options of container window
			tell opts
				set icon size to 128
				set text size to 16
				set arrangement to not arranged
			end tell
			set background picture of opts to file ".background:background.png"
			set position of item "Workrave" to {147, 202}
			set position of item "Applications" to {454, 202}
                        -- set position of item "Workrave.webloc" to {260, 150}

			-- SetFile -a V (invisible) is applied to these before this script
			-- runs, but that's not enough on its own: Finder installs that
			-- respect "show all files" preferences (common on dev machines)
			-- display invisible-flagged items too. Parking them far outside
			-- the visible window area is what actually keeps them off screen
			-- regardless of that preference, and also stops them from
			-- colliding with the explicitly-positioned icons above when
			-- Finder's default grid placement would otherwise put them there.
			try
				set position of item ".background" to {2000, 40}
			end try
			try
				set position of item ".fseventsd" to {2000, 140}
			end try
			try
				set position of item ".VolumeIcon.icns" to {2000, 240}
			end try

			update without registering applications
			tell container window
				open
				set the_window_id to id
			end tell
			update without registering applications
		end tell
		set bounds of window id the_window_id to {100, 100, 780, 600}
		--give the finder some time to write the .DS_Store file
		delay 5
	end tell
end run
