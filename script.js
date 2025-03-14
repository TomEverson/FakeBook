document.getElementById("togglePassword").addEventListener("click", function () {
    let passwordField = document.getElementById("password");
    if (passwordField.type === "password") {
        passwordField.type = "text";
        this.textContent = "🙈"; // Change icon to 'hide'
    } else {
        passwordField.type = "password";
        this.textContent = "👁️"; // Change icon to 'show'
    }
});