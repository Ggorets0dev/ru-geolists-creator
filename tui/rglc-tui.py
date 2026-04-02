import json
import getpass
import subprocess
from dataclasses import dataclass

from textual.app import App, ComposeResult
from textual.widgets import Header, Footer, ListView, ListItem, Label, Input, Button, Static, TabbedContent, TabPane, \
    Checkbox, RadioButton, RadioSet
from textual.containers import Container, Horizontal, Vertical, VerticalScroll
from textual.screen import Screen
from textual import on
from textual.reactive import reactive
from textual.validation import Length, Number, Regex

__VERSION__ = "0.1.0"
BIN_PATH = "../rglc-x86_64.AppImage"
CONFIG_PATH = f"/home/{getpass.getuser()}/.config/ru-geolists-creator/config.json"

class SourceEditScreen(Screen):
    @dataclass
    class ValidationMark:
        is_id_valid: bool = False
        is_storage_type_valid: bool = False
        is_inet_type_valid: bool = False

    """Screen for editing source"""
    def __init__(self, source_data: dict, on_save, is_new: bool = False):
        super().__init__()
        self.source_data = source_data.copy()
        self.on_save = on_save
        self.is_new = is_new
        self.valid_mark = self.ValidationMark

    def __apply_save_button_status(self):
        save_button = self.query_one("#save-btn", Button)
        save_button.disabled = not (self.valid_mark.is_id_valid and
                                    self.valid_mark.is_inet_type_valid and
                                    self.valid_mark.is_storage_type_valid)

    def __reset_valid_mark(self):
        self.valid_mark.is_id_valid = False
        self.valid_mark.is_storage_type_valid = False
        self.valid_mark.is_inet_type_valid = False

    def compose(self) -> ComposeResult:
        self.__reset_valid_mark()

        yield Header()
        # Use VerticalScroll so you can actually scroll down to the buttons
        with VerticalScroll(id="edit-form"):
            title = "New source" if self.is_new else f"Changing ID: {self.source_data.get('id')}"
            yield Label(title, classes="title")

            # --- Text Inputs ---
            yield Label("ID:", classes="field-label")
            yield Input(
                value=str(self.source_data.get("id", "")),
                id="edit-id",
                placeholder="123",
                validators=[Length(minimum=1), Number()]
            )

            yield Label("Section:", classes="field-label")
            yield Input(
                value=str(self.source_data.get("section", "")),
                id="edit-section",
                placeholder="my-awesome-section"
            )

            yield Label("URL:", classes="field-label")
            yield Input(
                value=str(self.source_data.get("url", "")),
                id="edit-url",
                placeholder="https://..."
            )

            # --- RadioSets (Replaces Inputs) ---
            yield Label("Storage Type:", classes="field-label")
            storage_val = self.source_data.get("storage_type", "")
            with RadioSet(id="edit-storage"):
                yield RadioButton("file_loc", value=storage_val == "file_loc")
                yield RadioButton("file_remote", value=storage_val == "file_remote")
                yield RadioButton("github_release", value=storage_val == "github_release")

            yield Label("Inet Type:", classes="field-label")
            inet_val = self.source_data.get("inet_type", "")
            with RadioSet(id="edit-inet-type"):
                yield RadioButton("domain", value=inet_val == "domain")
                yield RadioButton("ip", value=inet_val == "ip")

            # --- Checkbox (For binary types) ---
            yield Label("Preprocessing:", classes="field-label")
            yield Checkbox(
                "Extract Domains",
                value=self.source_data.get("preproc_type") == "extract_domains",
                id="edit-preproc-type"
            )

            # --- Action Buttons ---
            with Horizontal(classes="buttons-container"):
                yield Button("Save", variant="success", id="save-btn", disabled=True)
                if not self.is_new:
                    yield Button("Delete", variant="warning", id="delete-btn")
                yield Button("Cancel", variant="error", id="cancel-btn")
        yield Footer()

    @on(Input.Changed, "#edit-id")
    def check_id_validity(self, event: Input.Changed):
        self.valid_mark.is_id_valid = event.validation_result and event.validation_result.is_valid
        self.__apply_save_button_status()

    @on(RadioSet.Changed, "#edit-storage")
    def check_storage_validity(self, event: RadioSet.Changed):
        # If a button is pressed, it's valid
        self.valid_mark.is_storage_type_valid = event.radio_set.pressed_button is not None
        self.__apply_save_button_status()

    @on(RadioSet.Changed, "#edit-inet-type")
    def check_inet_type_validity(self, event: RadioSet.Changed):
        self.valid_mark.is_inet_type_valid = event.radio_set.pressed_button is not None
        self.__apply_save_button_status()

    @on(Button.Pressed, "#save-btn")
    def action_save(self):
        self.source_data.update({
            "id": int(self.query_one("#edit-id").value.strip()),
            "section": self.query_one("#edit-section").value.strip(),
            "url": self.query_one("#edit-url").value.strip(),
            "storage_type": str(self.query_one("#edit-storage").pressed_button.label),
            "preproc_type": "extract_domains" if self.query_one("#edit-preproc-type", Checkbox).value else "",
            "inet_type": str(self.query_one("#edit-inet-type").pressed_button.label)
        })
        if self.is_new and self.source_data.get("id") is None:
            self.source_data["id"] = self._generate_new_id()
        self.on_save(self.source_data)
        self.app.pop_screen()

    @on(Button.Pressed, "#delete-btn")
    def action_delete(self):
        self.on_save(None)
        self.app.pop_screen()

    @on(Button.Pressed, "#cancel-btn")
    def action_cancel(self):
        self.app.pop_screen()

    def _generate_new_id(self) -> int:
        sources = self.app.config_data.get("sources", [])
        ids = [int(s.get("id", 0)) for s in sources if str(s.get("id")).isdigit()]
        return max(ids, default=0) + 1

class RglcTUI(App):
    TITLE = "RGLC TUI"
    SUB_TITLE = f"v{__VERSION__}"

    CSS = """
    /* Main Screen Styles */
    #sources-list {
        background: $surface;
        margin: 1 2;
        border: solid $primary-darken-1;
        height: 1fr;
    }

    /* Individual List Items */
    ListItem {
        layout: horizontal;
        background: $boost;
        padding: 1 2;
        margin: 0 0 1 0;
        border-left: solid $accent;
        height: auto;
    }

    ListItem.-highlighted {
        background: $secondary;
    }

    ListItem:hover {
        background: $panel;
        border-left: solid $secondary;
    }

    /* Column styling inside the list item */
    .src-id {
        width: 8;
        text-style: bold;
        color: $accent;
    }

    .src-section {
        width: 20%;
        color: $secondary;
    }

    .src-url {
        width: 1fr;
        opacity: 0.8;
    }

    /* Form Styles (From your original code, polished) */
    SourceEditScreen {
        align: center middle;
    }

    #edit-form { 
        padding: 1 4; 
        border: tall $primary; 
        width: 70; 
        background: $surface;
    }

    .field-label { 
        margin-top: 1;           
        text-style: bold; 
        color: $accent;
    }

    Input {
        margin-bottom: 1; 
        border: tall $primary-lighten-1; 
    }

    RadioSet, Checkbox {
        margin: 1 0;
        background: $boost;
        border: round $primary-darken-1;
    }

    .title { 
        text-style: bold; 
        color: $accent; 
        width: 100%; 
        content-align: center middle;
        margin-bottom: 1;
        border-bottom: double $accent;
    }

    .buttons-container { 
        margin-top: 2;           
        height: 3; 
        align: center middle;
    }
    """

    BINDINGS = [
        ("q", "quit", "Exit"),
        ("s", "save_file", "Save JSON"),
        ("a", "add_source", "Add source"),
    ]

    config_data: dict = reactive({
        "apiToken": "",
        "bgpDumpPath": None,
        "dlcRootPath": "",
        "geoMgrBinaryPath": "",
        "singBoxBinaryPath": "",
        "sources": [],
        "presets": []
    })

    def get_about_text(self) -> str:
        """Вызывает ./rglc --about и возвращает результат."""
        try:
            # Запускаем процесс и получаем вывод
            result = subprocess.run(
                [BIN_PATH, "--about"],
                capture_output=True,
                text=True,
                check=True
            )
            return result.stdout
        except Exception as e:
            return f"Could not load info: {e}\nMake sure {BIN_PATH} exists and is executable."

    def compose(self) -> ComposeResult:
        yield Header()
        with TabbedContent():
            with TabPane("About", id="tab-about"):
                yield Vertical(
                    Static(self.get_about_text(), id="about-text"),
                )

            with TabPane("Settings", id="tab-main"):
                yield Vertical(
                    Label("API Token:"),
                    Input(id="apiToken", placeholder="github_pat_..."),
                    Label("BGP Dump Path:"),
                    Input(id="bgpDumpPath", placeholder="null или путь"),
                    Label("DLC Root Path:"),
                    Input(id="dlcRootPath"),
                    Label("Geo Manager Binary Path:"),
                    Input(id="geoMgrBinaryPath"),
                    Label("Sing-Box Binary Path:"),
                    Input(id="singBoxBinaryPath"),
                )
            with TabPane("Sources", id="tab-sources"):
                yield Label("List (Enter - edit, A - add):")
                yield ListView(id="sources-list")
        yield Footer()

    def on_mount(self):
        self.load_config()

    def load_config(self):
        try:
            with open("config.json", "r", encoding="utf-8") as f:
                data = json.load(f)
                if isinstance(data, dict):
                    self.config_data = data
        except (FileNotFoundError, json.JSONDecodeError):
            pass

        for key in ["apiToken", "bgpDumpPath", "dlcRootPath", "geoMgrBinaryPath", "singBoxBinaryPath"]:
            val = self.config_data.get(key)
            try:
                self.query_one(f"#{key}", Input).value = str(val) if val is not None else ""
            except: # На случай если виджет еще не готов
                pass

        self.refresh_list()

    def refresh_list(self):
        try:
            list_view = self.query_one("#sources-list", ListView)
            list_view.clear()
            for src in self.config_data.get("sources", []):
                label_text = f"ID: {src.get('id', '?')} | [{src.get('section', 'none')}] | {src.get('url', '')[:50]}"
                list_view.append(ListItem(Static(label_text)))
        except:
            pass

    def _sync_main_fields(self):
        for key in ["apiToken", "bgpDumpPath", "dlcRootPath", "geoMgrBinaryPath", "singBoxBinaryPath"]:
            val = self.query_one(f"#{key}", Input).value.strip()
            if key == "bgpDumpPath" and not val:
                self.config_data[key] = None
            else:
                self.config_data[key] = val

    def action_save_file(self):
        self._sync_main_fields()
        try:
            with open("config.json", "w", encoding="utf-8") as f:
                json.dump(self.config_data, f, indent=2, ensure_ascii=False)
            self.notify("Config is saved!")
        except Exception as e:
            self.notify(f"Ошибка сохранения: {e}", severity="error")

    def action_add_source(self):
        def callback(updated):
            if updated:
                self.config_data["sources"].append(updated)
                self.refresh_list()
        self.push_screen(SourceEditScreen({}, callback, is_new=True))

    @on(ListView.Selected)
    def handle_selection(self, event: ListView.Selected):
        idx = event.list_view.index
        if idx is None or idx >= len(self.config_data["sources"]): return

        source = self.config_data["sources"][idx]
        def callback(updated):
            if updated is None:
                self.config_data["sources"].pop(idx)
            else:
                self.config_data["sources"][idx] = updated
            self.refresh_list()
        self.push_screen(SourceEditScreen(source, callback))

if __name__ == "__main__":
    RglcTUI().run()
