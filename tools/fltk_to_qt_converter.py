#!/usr/bin/env python3
"""
FLTK to Qt Callback Converter
Automatically converts FLTK callback patterns to Qt signal/slot patterns
"""

import re
import sys
from pathlib import Path


class FltkToQtConverter:
    def __init__(self):
        self.fltk_to_qt_widgets = {
            'Fl_Button': 'QPushButton',
            'Fl_Check_Button': 'QCheckBox',
            'Fl_Round_Button': 'QRadioButton',
            'Fl_Choice': 'QComboBox',
            'Fl_Choice_': 'QComboBox',
            'Fl_Value_Input': 'QSpinBox',
            'Fl_Input': 'QLineEdit',
            'Fl_Input_': 'QLineEdit',
            'Fl_Text_Editor': 'QTextEdit',
            'Fl_Menu_Button': 'QMenu',
            'Fl_Box': 'QLabel',
            'Fl_Tabs': 'QTabWidget',
            'Fl_Group': 'QWidget',
            'Fl_Double_Window': 'QMainWindow',
            'Fl_Window': 'QDialog',
        }
        
        self.fltk_to_qt_signals = {
            'QPushButton': 'clicked',
            'QCheckBox': 'toggled',
            'QRadioButton': 'toggled',
            'QComboBox': 'currentIndexChanged',
            'QSpinBox': 'valueChanged',
            'QLineEdit': 'textChanged',
            'QTextEdit': 'textChanged',
        }
    
    def convert_header(self, fltk_header_path, qt_header_path):
        """Convert FLTK header file to Qt header file"""
        print(f"Converting header: {fltk_header_path} -> {qt_header_path}")
        
        with open(fltk_header_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Replace includes
        content = self._replace_includes(content)
        
        # Replace widget types
        content = self._replace_widget_types(content)
        
        # Convert callback declarations to slots
        content = self._convert_callback_declarations(content)
        
        # Add Q_OBJECT macro if it's a QObject-derived class
        content = self._add_qobject_macro(content)
        
        with open(qt_header_path, 'w', encoding='utf-8') as f:
            f.write(content)
        
        print(f"Header conversion complete!")
    
    def convert_implementation(self, fltk_cpp_path, qt_cpp_path):
        """Convert FLTK implementation file to Qt implementation file"""
        print(f"Converting implementation: {fltk_cpp_path} -> {qt_cpp_path}")
        
        with open(fltk_cpp_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Replace includes
        content = self._replace_includes(content)
        
        # Extract all callback registrations
        callbacks = self._extract_callbacks(content)
        
        # Generate connectSignals function
        connect_signals = self._generate_connect_signals(callbacks)
        
        # Convert callback static wrappers to slots
        content = self._convert_callback_wrappers(content)
        
        # Add connectSignals function
        content = self._add_connect_signals_function(content, connect_signals)
        
        with open(qt_cpp_path, 'w', encoding='utf-8') as f:
            f.write(content)
        
        print(f"Implementation conversion complete!")
        print(f"Generated {len(callbacks)} signal/slot connections")
    
    def _replace_includes(self, content):
        """Replace FLTK includes with Qt includes"""
        # FLTK includes
        fltk_includes = [
            '#include <FL/.*>',
            '#include <Fl/.*>',
        ]
        
        for pattern in fltk_includes:
            content = re.sub(pattern, '', content)
        
        # Add Qt includes if not present
        qt_includes = '''#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QGroupBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QClipboard>
'''
        
        # Add Qt includes after the first local include
        if '#include "../inc/' in content:
            parts = content.split('#include "../inc/', 1)
            if len(parts) == 2:
                first_include = parts[1].split('\n', 1)[0]
                content = parts[0] + '#include "../inc/' + first_include + '\n\n' + qt_includes + '\n' + parts[1].split('\n', 1)[1]
        
        return content
    
    def _replace_widget_types(self, content):
        """Replace FLTK widget types with Qt equivalents"""
        for fltk_widget, qt_widget in self.fltk_to_qt_widgets.items():
            # Replace widget declarations
            content = re.sub(rf'\b{fltk_widget}\b\s*\*', f'{qt_widget} *', content)
        
        return content
    
    def _convert_callback_declarations(self, content):
        """Convert FLTK callback declarations to Qt slots"""
        # Find static callback declarations
        # Pattern: static void cb_name(Fl_Widget *, void *);
        pattern = r'static void (cb_\w+)\(Fl_Widget \*, void \*\);'
        
        # Replace with slot declarations
        def replace_callback(match):
            callback_name = match.group(1)
            return f'void {callback_name}();'
        
        # Move to public slots section
        content = re.sub(pattern, replace_callback, content)
        
        # Find the public section and add "public slots:" if not present
        if 'public:' in content and 'public slots:' not in content:
            # Add after first public:
            content = content.replace('public:', 'public:\n\npublic slots:', 1)
        
        return content
    
    def _add_qobject_macro(self, content):
        """Add Q_OBJECT macro to class declaration"""
        # Find class declaration
        # Pattern: class ClassName : public QMainWindow
        pattern = r'(class\s+\w+\s*:\s*public\s+Q\w+[^\{]*\{)'
        
        def add_macro(match):
            class_decl = match.group(1)
            return class_decl + '\n    Q_OBJECT\n'
        
        # Only add if not already present
        if 'Q_OBJECT' not in content:
            content = re.sub(pattern, add_macro, content)
        
        return content
    
    def _extract_callbacks(self, content):
        """Extract all callback registrations from FLTK code"""
        # Pattern: widget->callback(cb_name, this);
        pattern = r'(\w+)->callback\((cb_\w+),\s*this\);'
        
        matches = re.findall(pattern, content)
        
        callbacks = []
        for widget_name, callback_name in matches:
            callbacks.append({
                'widget': widget_name,
                'callback': callback_name,
                'method': callback_name
            })
        
        return callbacks
    
    def _generate_connect_signals(self, callbacks):
        """Generate Qt connect statements"""
        connect_statements = []
        
        for cb in callbacks:
            widget = cb['widget']
            method = cb['method']
            
            # Determine widget type from widget name
            qt_widget_type = self._guess_widget_type(widget)
            signal = self.fltk_to_qt_signals.get(qt_widget_type, 'clicked')
            
            # Special cases
            if 'check' in widget.lower():
                signal = 'toggled'
            elif 'choice' in widget.lower() or 'combo' in widget.lower():
                signal = 'currentIndexChanged'
            elif 'input' in widget.lower() and 'button' not in widget.lower():
                signal = 'textChanged'
            elif 'button' in widget.lower():
                signal = 'clicked'
            
            # Generate connect statement
            if signal in ['toggled', 'clicked']:
                connect_stmt = f'    connect({widget}, &{qt_widget_type}::{signal}, this, &QtToolkit::{method});'
            else:
                connect_stmt = f'    connect({widget}, QOverload<int>::of(&{qt_widget_type}::{signal}), this, &QtToolkit::{method});'
            
            connect_statements.append(connect_stmt)
        
        return connect_statements
    
    def _guess_widget_type(self, widget_name):
        """Guess Qt widget type from widget name"""
        widget_lower = widget_name.lower()
        
        if 'button' in widget_lower and 'check' not in widget_lower:
            return 'QPushButton'
        elif 'check' in widget_lower:
            return 'QCheckBox'
        elif 'radio' in widget_lower:
            return 'QRadioButton'
        elif 'choice' in widget_lower or 'combo' in widget_lower:
            return 'QComboBox'
        elif 'input' in widget_lower and 'text' not in widget_lower:
            return 'QSpinBox'
        elif 'input' in widget_lower or 'edit' in widget_lower:
            return 'QLineEdit'
        elif 'editor' in widget_lower:
            return 'QTextEdit'
        elif 'box' in widget_lower or 'label' in widget_lower:
            return 'QLabel'
        else:
            return 'QPushButton'  # Default fallback
    
    def _convert_callback_wrappers(self, content):
        """Convert FLTK static callback wrappers to Qt slots"""
        # Pattern: void ClassName::cb_name(Fl_Widget *, void *w) { ((ClassName *)w)->cb_name(); }
        pattern = r'void\s+(\w+)::(cb_\w+)\(Fl_Widget\s*\*,\s*void\s*\*w\)\s*\{\s*\(\((\w+)\s*\*\)w\)->\2\(\);\s*\}'
        
        # Remove these wrappers entirely (Qt doesn't need them)
        content = re.sub(pattern, '', content)
        
        # Also remove the inline wrapper declarations
        pattern2 = r'void\s+(\w+)::(cb_\w+)\(Fl_Widget\s*\*,\s*void\s*\*w\)\s*\n\s*\{\s*\n\s*\(\((\w+)\s*\*\)w\)->\2\(\);\s*\n\s*\}'
        content = re.sub(pattern2, '', content)
        
        return content
    
    def _add_connect_signals_function(self, content, connect_signals):
        """Add connectSignals() function to implementation"""
        if not connect_signals:
            return content
        
        connect_function = '\nvoid QtToolkit::connectSignals()\n{\n'
        connect_function += '\n'.join(connect_signals)
        connect_function += '\n}\n\n'
        
        # Insert after constructor
        # Pattern: ClassName::ClassName(...) { ... }
        pattern = r'(QtToolkit::QtToolkit[^\{]*\{[^\}]*\})'
        
        def add_after_constructor(match):
            constructor = match.group(1)
            return constructor + '\n' + connect_function
        
        content = re.sub(pattern, add_after_constructor, content, count=1)
        
        return content
    
    def analyze_file(self, file_path):
        """Analyze FLTK file and print conversion info"""
        print(f"\n=== Analyzing {file_path} ===\n")
        
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Find all callback registrations
        callbacks = self._extract_callbacks(content)
        
        print(f"Found {len(callbacks)} callback registrations:\n")
        for cb in callbacks:
            print(f"  {cb['widget']} -> {cb['callback']}")
        
        # Find all widget declarations
        widget_pattern = r'(Fl_\w+)\s*\*(\w+);'
        widgets = re.findall(widget_pattern, content)
        
        print(f"\nFound {len(widgets)} FLTK widget declarations:\n")
        for fltk_type, name in widgets:
            qt_type = self.fltk_to_qt_widgets.get(fltk_type, 'Unknown')
            print(f"  {name}: {fltk_type} -> {qt_type}")
        
        print()


def main():
    converter = FltkToQtConverter()
    
    if len(sys.argv) < 2:
        print("FLTK to Qt Callback Converter")
        print("\nUsage:")
        print("  python fltk_to_qt_converter.py analyze <file>")
        print("  python fltk_to_qt_converter.py convert-header <input.h> <output.h>")
        print("  python fltk_to_qt_converter.py convert-impl <input.cpp> <output.cpp>")
        print("\nExamples:")
        print("  python fltk_to_qt_converter.py analyze ../src/toolkit.cpp")
        print("  python fltk_to_qt_converter.py convert-header ../inc/toolkit.h ../inc/toolkit_qt.h")
        print("  python fltk_to_qt_converter.py convert-impl ../src/toolkit.cpp ../src/toolkit_qt.cpp")
        return
    
    command = sys.argv[1]
    
    if command == 'analyze':
        if len(sys.argv) < 3:
            print("Error: Please provide file path")
            return
        converter.analyze_file(sys.argv[2])
    
    elif command == 'convert-header':
        if len(sys.argv) < 4:
            print("Error: Please provide input and output file paths")
            return
        converter.convert_header(sys.argv[2], sys.argv[3])
    
    elif command == 'convert-impl':
        if len(sys.argv) < 4:
            print("Error: Please provide input and output file paths")
            return
        converter.convert_implementation(sys.argv[2], sys.argv[3])
    
    else:
        print(f"Error: Unknown command '{command}'")


if __name__ == '__main__':
    main()
