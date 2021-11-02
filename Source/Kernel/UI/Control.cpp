#include <Kernel/UI/Control.hpp>
#include <Kernel/Core/Kernel.hpp>

namespace PMOS
{
    namespace UI
    {
        // function to convert null type to container type
        Container* GetParent(void* parent) { return ((Container*)parent); }

        // create button
        Button* CreateButton(int x, int y, char* text) { return CreateButton(x, y, text, nullptr, nullptr); }
        Button* CreateButton(int x, int y, char* text, char* name) { return CreateButton(x, y, text, name, nullptr); }
        Button* CreateButton(int x, int y, char* text, Container* parent) { return CreateButton(x, y, text, nullptr, parent); }
        Button* CreateButton(int x, int y, char* text, char* name, Container* parent)
        {
            if (parent == nullptr)
            {
                Button* obj = new Button(x, y, text);
                HeapEntry* entry = Kernel::MemoryMgr.GetEntryFromPtr(obj);
                if (entry != nullptr) { entry->Type = (byte)AllocationType::UI; }
                if (name != nullptr) { obj->SetName(name); } else { obj->SetName(text); }
                Kernel::Debug.Info("Created new button: 0x%8x", (uint)obj);
                return obj;
            }
            else
            {
                Button* obj = new Button(x, y, text, parent);
                HeapEntry* entry = Kernel::MemoryMgr.GetEntryFromPtr(obj);
                if (entry != nullptr) { entry->Type = (byte)AllocationType::UI; }
                if (name != nullptr) { obj->SetName(name); } else { obj->SetName(text); }
                Kernel::Debug.Info("Created new button: 0x%8x", (uint)obj);
                return obj;
            }
        }

        // create container
        Container* CreateContainer(int x, int y, int w, int h) { return CreateContainer(x, y, w, h, nullptr); }
        Container* CreateContainer(int x, int y, int w, int h, char* name)
        {
            Container* obj = new Container(x, y, w, h);
            HeapEntry* entry = Kernel::MemoryMgr.GetEntryFromPtr(obj);
            if (entry != nullptr) { entry->Type = (byte)AllocationType::UI; }
            if (name != nullptr) { obj->SetName(name); }
            Kernel::Debug.Info("Created new container: 0x%8x", (uint)obj);
            return obj;
        }

        // create window
        Window* CreateWindow(int x, int y, int w, int h, char* name) { return CreateWindow(x, y, w, h, name, name, nullptr); }
        Window* CreateWindow(int x, int y, int w, int h, char* title, char* name) { return CreateWindow(x, y, w, h, title, name, nullptr); }
        Window* CreateWindow(int x, int y, int w, int h, char* title, char* name, char* args)
        {
            Window* obj = new Window(x, y, w, h, title, name, args);
            HeapEntry* entry = Kernel::MemoryMgr.GetEntryFromPtr(obj);
            if (entry != nullptr) { entry->Type = (byte)AllocationType::UI; }
            Kernel::Debug.Info("Created new window: 0x%8x", (uint)obj);
            return obj;
        }

        #pragma region "Control Base"

        Control::Control() { }

        Control::Control(int x, int y, int w, int h, ControlType type)
        {
            // set dimensions
            this->SetBounds(x, y, w, h);

            // set properties
            this->Parent = nullptr;
            this->Type = type;
            this->TextAlign = Alignment::MiddleCenter;

            // set strings
            this->SetText("CONTROL");
            this->SetName("CONTROL");
            this->SetTag("NULL");

            // set style
            this->SetStyle((VisualStyle*)&ButtonStyle);

            // set flags
            this->Flags.CanClick = true;
            this->Flags.CanFocus = true;
            this->Flags.CanRefresh = false;
            this->Flags.Visible  = true;
            this->Flags.Enabled  = true;
            this->Flags.Focused  = true;
            this->Flags.Child    = false;
        }

        Control::Control(int x, int y, int w, int h, ControlType type, void* parent)
        {
            // set dimensions
            this->SetBounds(x, y, w, h);

            // set properties
            this->Parent = parent;
            this->Type = type;
            this->TextAlign = Alignment::MiddleCenter;

            // set strings
            this->SetText("CONTROL");
            this->SetName("CONTROL");
            this->SetTag("NULL");

            // set style
            this->SetStyle((VisualStyle*)&ButtonStyle);

            // set flags
            this->Flags.CanClick = true;
            this->Flags.CanFocus = true;
            this->Flags.CanRefresh = true;
            this->Flags.Visible  = true;
            this->Flags.Enabled  = true;
            this->Flags.Focused  = true;
            this->Flags.Child    = false;
        }

        void Control::Dispose()
        {
            OnDestroy();
            if (Text != nullptr) { Kernel::MemoryMgr.Free(Text); }
            if (Name != nullptr) { Kernel::MemoryMgr.Free(Name); }
            if (Tag != nullptr)  { Kernel::MemoryMgr.Free(Tag); }
        }

        void Control::InvokeRefresh()
        {
            Flags.CanRefresh = true;
            Refresh();
        }

        void Control::OnCreate() { }
        void Control::OnDestroy() { }

        void Control::Update()
        {
            if (Flags.Enabled)
            {
                int mx = Kernel::Mouse->GetX();
                int my = Kernel::Mouse->GetY();

                // not a child
                if (Parent == nullptr)
                {
                    // check if mouse is hovering over control
                    if (RectangleContains(Bounds, mx, my))
                    {
                        // check hover state
                        if (!MSFlags.Hover)
                        {
                            MSFlags.Hover = true;
                            OnMouseEnter();
                        }

                        // hover event
                        OnMouseHover();

                        if (Kernel::Mouse->IsLeftPressed()) { Flags.Focused = true; }

                        // check click state
                        if (Kernel::Mouse->IsLeftPressed() && !MSFlags.Clicked)
                        {
                            OnMouseDown();
                            MSFlags.Down = true;
                            MSFlags.Clicked = true;
                            MSFlags.UnClicked = false;
                        }

                        // check click release state
                        if (!Kernel::Mouse->IsLeftPressed() && !MSFlags.UnClicked)
                        {
                            MSFlags.Down = false;
                            if (MSFlags.Clicked)
                            {
                                OnClick();
                                OnMouseUp();
                                MSFlags.UnClicked = true;
                                MSFlags.Clicked = false;
                            }
                        }
                    }
                    // mouse is not hovering over control
                    else
                    {
                        if (MSFlags.Hover)
                        {
                            MSFlags.Hover = false;
                            OnMouseLeave();
                            InvokeRefresh();
                        }
                        MSFlags.Down      = false;
                        MSFlags.Clicked   = false;
                        MSFlags.UnClicked = false;
                        MSFlags.Up        = true;
                        if (Kernel::Mouse->IsLeftPressed()) { Flags.Focused = false; }
                    }
                }
                // child of parent
                else
                {
                    if (GetParent(Parent)->Flags.Focused)
                    {
                        if (Kernel::Mouse->IsLeftPressed()) { Flags.Focused = true; }

                        if (RectangleContains(GetParent(Parent)->Bounds.X + Bounds.X, GetParent(Parent)->Bounds.Y + Bounds.Y, Bounds.Width, Bounds.Height, mx, my))
                        {
                            if (!MSFlags.Hover)
                            {
                                MSFlags.Hover = true;
                                OnMouseEnter();
                            }

                            OnMouseHover();
                            if (Kernel::Mouse->IsLeftPressed() && !MSFlags.Clicked)
                            {
                                OnMouseDown();
                                MSFlags.Down = true;
                                MSFlags.Clicked = true;
                                MSFlags.UnClicked = false;
                            }
                            if (!Kernel::Mouse->IsLeftPressed() && !MSFlags.UnClicked)
                            {
                                MSFlags.Down = false;
                                if (MSFlags.Clicked)
                                {
                                    OnClick();
                                    OnMouseUp();
                                    MSFlags.UnClicked = true;
                                    MSFlags.Clicked = false;
                                }
                            }
                        }
                        else
                        {
                            if (MSFlags.Hover)
                            {
                                MSFlags.Hover = false;
                                OnMouseLeave();
                                InvokeRefresh();
                            }

                            MSFlags.Down      = false;
                            MSFlags.Clicked   = false;
                            MSFlags.UnClicked = false;
                            MSFlags.Up        = true;
                            if (Kernel::Mouse->IsLeftPressed()) { Flags.Focused = false; }
                        }
                    }
                }
            }
        }

        void Control::Draw()
        {

        }

        void Control::Refresh()
        {
            if (Flags.Visible && Flags.CanRefresh)
            {
                Draw();
                Flags.CanRefresh = false;
            }
        }
        
        void Control::OnClick()
        {
            if (Click != nullptr) { Click(Parent, this); }
            if (Flags.Child)
            {
                if (Parent != nullptr) { GetParent(Parent)->InvokeRefresh(); }
            }
            else { InvokeRefresh(); }
            Kernel::Debug.Info("Event 'OnClick'        invoked : ", Name);
        }

        void Control::OnMouseDown()
        {
            MSFlags.Down = true;
            if (MouseDown != nullptr) { MouseDown(Parent, this); }
            if (Flags.Child)
            {
                if (Parent != nullptr) { GetParent(Parent)->InvokeRefresh(); }
            }
            else { InvokeRefresh(); }
            Kernel::Debug.Info("Event 'OnMouseDown'    invoked : ", Name);
        }

        void Control::OnMouseUp()
        {
            MSFlags.Down = false;
            if (MouseUp != nullptr) { MouseUp(Parent, this); }
            if (Flags.Child)
            {
                if (Parent != nullptr) { GetParent(Parent)->InvokeRefresh(); }
            }
            else { InvokeRefresh(); }
            Kernel::Debug.Info("Event 'OnMouseUp'      invoked : ", Name);
        }

        void Control::OnMouseHover()
        {
            if (MouseHover != nullptr) { MouseHover(Parent, this); }
        }

        void Control::OnMouseLeave()
        {
            if (MouseLeave != nullptr) { MouseLeave(Parent, this); }
            Kernel::Debug.Info("Event 'OnMouseLeave'   invoked : ", Name);
        }

        void Control::OnMouseEnter()
        {
            if (Flags.Child)
            {
                if (Parent != nullptr) { GetParent(Parent)->InvokeRefresh(); }
            }
            else { InvokeRefresh(); }
            Kernel::Debug.Info("Event 'OnMouseEnter'   invoked : ", Name);
        }


        void Control::SetPosition(int x, int y) { Bounds.X = x; Bounds.Y = y; }
        void Control::SetPosition(Point pos) { Bounds.X = pos.X; Bounds.Y = pos.Y; }
        void Control::SetSize(int w, int h) { Bounds.Width = w; Bounds.Height = h; }
        void Control::SetSize(Point size) { Bounds.Width = size.X; Bounds.Height = size.Y; }
        void Control::SetBounds(int x, int y, int w, int h) { Bounds.X = x; Bounds.Y = y; Bounds.Width = w; Bounds.Height = h; }
        void Control::SetBounds(Point pos, Point size) { SetBounds(pos.X, pos.Y, size.X, size.Y); }
        void Control::SetBounds(Rectangle bounds) { SetBounds(bounds.X, bounds.Y, bounds.Width, bounds.Height); }

        void Control::SetText(char* text)
        {
            Kernel::Debug.WriteLine("TEXT: %s", text);
            if (Text != nullptr) { Kernel::MemoryMgr.Free(Text); }
            if (text != nullptr)
            {
                // input is blank
                if (String::Length(text) == 0) { Text = (char*)Kernel::MemoryMgr.Allocate(1); Text[0] = 0; return; }
                // input has value - copy it
                Text = (char*)Kernel::MemoryMgr.Allocate(String::Length(text) + 1, true, AllocationType::String);
                String::Copy(Text, text);
            }
        }

        void Control::SetName(char* name)
        {
            if (Name != nullptr) { Kernel::MemoryMgr.Free(Name); }
            if (name != nullptr)
            {
                // input is blank
                if (String::Length(name) == 0) { Text = (char*)Kernel::MemoryMgr.Allocate(1); Name[0] = 0; return; }
                // input has value - copy it
                Name = (char*)Kernel::MemoryMgr.Allocate(String::Length(name) + 1, true, AllocationType::String);
                String::Copy(Name, name);
            }
        }

        void Control::SetTag(char* tag)
        {
            if (Tag != nullptr) { Kernel::MemoryMgr.Free(Tag); }
            if (tag != nullptr)
            {
                // input is blank
                if (String::Length(tag) == 0) { Text = (char*)Kernel::MemoryMgr.Allocate(1); Tag[0] = 0; return; }
                // input has value - copy it
                Tag = (char*)Kernel::MemoryMgr.Allocate(String::Length(tag) + 1, true, AllocationType::String);
                String::Copy(Tag, tag);
            }
        }

        void Control::SetStyle(VisualStyle* style) { CopyStyle(&Style, style); }

        void Control::Toggle(bool state) { Flags.Enabled = state; }

        #pragma endregion

    }
}