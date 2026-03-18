# **Implementation Guide: Secure Password Update Field (Placeholder Pattern)**

## **1\. Architectural Concept & Data Flow**

This document outlines the canonical "Placeholder Pattern" for a password update field (typically found in user settings).

**The Core Rule:** The frontend NEVER possesses the user's current password.

* **NEVER** put the actual password in the HTML.  
* **NEVER** put dummy characters (like ••••••••) in the value attribute. Doing so risks submitting those dummy characters to the server and overwriting the user's actual password.

**The Solution:** The server dictates the visual state using the placeholder attribute, while leaving the value attribute strictly empty ("").

### **The Two States**

1. **Password Exists:** The server renders placeholder="••••••••". The CSS styles this to look like a masked password.  
2. **No Password Exists (e.g., OAuth users):** The server renders placeholder="Create a password".

Because these are just placeholders, the browser automatically clears them the moment the user starts typing. If the user submits the form without typing, the payload sends "" (empty string), which the backend safely ignores.

## **2\. HTML Structure**

The markup requires a wrapper to position the toggle button over the input. The button MUST be type="button" so it doesn't accidentally trigger a form submission.

\<\!-- Example of State 1: Password Exists \--\>  
\<div class="password-field-container"\>  
  \<label for="new-password"\>Update Password\</label\>  
    
  \<div class="password-input-wrapper"\>  
    \<\!-- Notice value is empty. Placeholder holds the visual state. \--\>  
    \<input   
      type="password"   
      id="new-password"   
      name="new-password"   
      value=""   
      placeholder="••••••••"   
      class="existing-password-mask"  
      autocomplete="new-password"  
    \>  
      
    \<\!-- Toggle Button. Hidden/disabled by default via CSS/JS until user types \--\>  
    \<button type="button" class="password-toggle-btn" aria-label="Show password" disabled\>  
      \<\!-- Eye Icon (Default: visible/open eye) \--\>  
      \<svg class="eye-open" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"\>  
        \<path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"\>\</path\>  
        \<circle cx="12" cy="12" r="3"\>\</circle\>  
      \</svg\>  
      \<\!-- Eye Crossed Icon (Hidden by default) \--\>  
      \<svg class="eye-crossed" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" style="display: none;"\>  
        \<path d="M17.94 17.94A10.07 10.07 0 0 1 12 20c-7 0-11-8-11-8a18.45 18.45 0 0 1 5.06-5.94M9.9 4.24A9.12 9.12 0 0 1 12 4c7 0 11 8 11 8a18.5 18.5 0 0 1-2.16 3.19m-6.72-1.07a3 3 0 1 1-4.24-4.24"\>\</path\>  
        \<line x1="1" y1="1" x2="23" y2="23"\>\</line\>  
      \</svg\>  
    \</button\>  
  \</div\>  
\</div\>

*Note: For State 2 (No Password Exists), simply change the placeholder to "Create a password" and remove the existing-password-mask class.*

## **3\. CSS Styling**

The CSS handles two main tasks: absolutely positioning the toggle button inside the relative wrapper, and making the dot placeholder look authentic.

.password-field-container {  
  display: flex;  
  flex-direction: column;  
  max-width: 320px;  
  font-family: system-ui, sans-serif;  
}

.password-input-wrapper {  
  position: relative;  
  display: flex;  
  align-items: center;  
  margin-top: 4px;  
}

.password-input-wrapper input {  
  width: 100%;  
  padding: 10px 40px 10px 12px; /\* 40px right padding prevents text hiding behind the icon \*/  
  border: 1px solid \#ccc;  
  border-radius: 6px;  
  font-size: 16px;  
}

/\* Specific styling for the dots placeholder.   
  Letter-spacing makes it look like a real masked password.  
\*/  
input.existing-password-mask::placeholder {  
  color: \#888;  
  letter-spacing: 4px;  
  font-size: 14px;  
  transform: translateY(-2px); /\* Adjust vertical alignment of dots if necessary \*/  
}

/\* General placeholder styling for "Create a password" \*/  
input:not(.existing-password-mask)::placeholder {  
  color: \#888;  
  letter-spacing: normal;  
}

/\* Toggle Button Positioning \*/  
.password-toggle-btn {  
  position: absolute;  
  right: 10px;  
  background: transparent;  
  border: none;  
  cursor: pointer;  
  padding: 4px;  
  display: flex;  
  align-items: center;  
  justify-content: center;  
  color: \#666;  
  transition: color 0.2s ease;  
}

.password-toggle-btn:disabled {  
  opacity: 0.4;  
  cursor: not-allowed;  
}

.password-toggle-btn svg {  
  width: 20px;  
  height: 20px;  
}

## **4\. JavaScript Logic**

The JavaScript must enforce the rule that **the user cannot "unhide" the placeholder**. The toggle button should only function (and visually activate) when input.value.length \> 0\.

document.addEventListener('DOMContentLoaded', () \=\> {  
  const input \= document.getElementById('new-password');  
  const toggleBtn \= document.querySelector('.password-toggle-btn');  
  const eyeOpen \= toggleBtn.querySelector('.eye-open');  
  const eyeCrossed \= toggleBtn.querySelector('.eye-crossed');

  // 1\. Listen for typing to enable/disable the toggle button  
  input.addEventListener('input', () \=\> {  
    if (input.value.length \> 0\) {  
      toggleBtn.disabled \= false;  
    } else {  
      // If user deletes their input, disable button and revert to masked state  
      toggleBtn.disabled \= true;  
      input.type \= 'password';  
      updateIconState(false);  
    }  
  });

  // 2\. Handle the visibility toggle click  
  toggleBtn.addEventListener('click', () \=\> {  
    // Failsafe: Do nothing if the input is actually empty (showing placeholder)  
    if (input.value.length \=== 0\) return;

    const isCurrentlyPassword \= input.type \=== 'password';  
      
    // Toggle the input type  
    input.type \= isCurrentlyPassword ? 'text' : 'password';  
      
    // Update visuals and accessibility  
    updateIconState(isCurrentlyPassword);  
  });

  // Helper function to manage icons and ARIA attributes  
  function updateIconState(isShowingText) {  
    if (isShowingText) {  
      eyeOpen.style.display \= 'none';  
      eyeCrossed.style.display \= 'block';  
      toggleBtn.setAttribute('aria-label', 'Hide password');  
    } else {  
      eyeOpen.style.display \= 'block';  
      eyeCrossed.style.display \= 'none';  
      toggleBtn.setAttribute('aria-label', 'Show password');  
    }  
  }  
});

## **5\. Security Checklist for the Agent**

* \[ \] Ensure the backend validates the current session before accepting a new password.  
* \[ \] Ensure the form tag does NOT include dummy data in the password payload if the user submits without typing.  
* \[ \] Include autocomplete="new-password" to prevent password managers from incorrectly autofilling the user's *current* password into this specific field upon page load.