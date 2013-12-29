on run -- for testing in script editor
	tell application "Finder"
		tell disk "Workrave"
			open
			tell container window
				set current view to icon view
				set toolbar visible to false
				set statusbar visible to false
				set the bounds to {30, 50, 579, 600}
			end tell
			close
			set opts to the icon view options of container window
			tell opts
				set icon size to 64
				set arrangement to not arranged
			end tell
			set background picture of opts to file ".background:background.png"
			set position of item "Workrave" to {100, 40}
			set position of item "Applications" to {420, 40}
                        -- set position of item "Workrave.webloc" to {260, 150}
			
			update without registering applications
			tell container window
				open
				set the_window_id to id
			end tell
			update without registering applications
		end tell
		set bounds of window id the_window_id to {30, 50, 575, 415}
		--give the finder some time to write the .DS_Store file
		delay 5
	end tell
end run
