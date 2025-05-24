# CSS Styling

Workrave supports customization of colors through CSS. You can change the colors of the prelude window (the window that appears before a break) and other UI elements.

To customize Workrave with CSS:

1. Create a file named `user.css` in your user config directory:

   - Windows: `%APPDATA%\workrave\user.css` (usually `C:\Users\<YourUsername>\AppData\Roaming\workrave\user.css`)
   - Linux: `~/.config/workrave/user.css`

2. Add CSS rules to customize the appearance. For example:

   ```css

    .workrave-flash-warn {
      color: #0055ff;
    }
    .workrave-flash-alert {
      color: purple;
    }

    .workrave-timebar {
      color: black;
      background-color: #777777;
    }
    .workrave-timebar-active {
      color: black;
      background-color: lightblue;
    }
    .workrave-timebar-inactive {
      color: black;
      background-color: lightgreen;
    }
    .workrave-timebar-inactive#rest-break{
      color: black;
      background-color: mediumspringgreen;
    }
    .workrave-timebar-overdue {
      color: black;
      background-color: orange;
    }
    .workrave-timebar-inactive-over-active {
      color: black;
      background-color: #00d4b2;
    }
    .workrave-timebar-inactive-over-overdue {
      color: black;
      background-color: lightgreen;
    }
    .workrave-timebar-inactive-over-inactive {
      color: black;
      background-color: #00d4b2;
    }

   ```

3. Restart Workrave for the changes to take effect.
