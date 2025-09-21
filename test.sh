
echo "Fixing const char** in ChromiumOS files..."

# Target specific files that are likely to have this issue
for file in driver/usb_mux/*.c ; do
    if [ -f "$file" ] && grep -q "int argc, const char \*\*argv" "$file"; then
        echo "Fixing: $file"
        cp "$file" "$file.backup"
        sed -i 's/int argc, const char \*\*argv/int argc, char **argv/g' "$file"
    fi
done

echo "Done!"
