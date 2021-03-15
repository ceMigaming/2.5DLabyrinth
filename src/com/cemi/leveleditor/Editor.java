package com.cemi.leveleditor;

import java.awt.Component;
import java.awt.Font;
import java.awt.Insets;
import java.awt.event.KeyEvent;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.util.Scanner;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.SwingUtilities;

public class Editor {

	static String currentMode = ".";
	static String outputString = "";

	public static void main(String[] args) {

		JPanel panel = new JPanel();
		JFrame frame = new JFrame();
		frame.setSize(640, 480);
		frame.setResizable(false);
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		frame.setVisible(true);
		frame.add(panel);
		frame.setName("Edytor poziomów");

		panel.setLayout(null);

		JLabel currentOptionsLabel = new JLabel();
		JLabel currentOptionLabel = new JLabel();

		JMenuBar menuBar = new JMenuBar();

		JMenu fileMenu = new JMenu("Plik");
		fileMenu.setMnemonic(KeyEvent.VK_F);

		JMenuItem newItem = new JMenuItem("Nowy");
		newItem.setMnemonic(KeyEvent.VK_N);
		newItem.setDisplayedMnemonicIndex(0);
		newItem.setToolTipText("Utwórz now¹ mapê");
		newItem.addActionListener((event) -> createLevel(frame, panel, currentOptionLabel, 16, 16));
		fileMenu.add(newItem);

		JMenuItem loadItem = new JMenuItem("Otwórz");
		loadItem.setMnemonic(KeyEvent.VK_N);
		loadItem.setDisplayedMnemonicIndex(0);
		loadItem.setToolTipText("Otwórz mapê");
		loadItem.addActionListener((event) -> loadMap(frame, panel, currentOptionLabel, 16, 16, new File("map.txt")));
		fileMenu.add(loadItem);

		JMenuItem saveItem = new JMenuItem("Zapisz");
		saveItem.setMnemonic(KeyEvent.VK_S);
		saveItem.setToolTipText("Zapisz mapê");
		saveItem.addActionListener((event) -> saveMap(frame, 16, 16));
		fileMenu.add(saveItem);
		
		JMenuItem cleanItem = new JMenuItem("Wyczyœæ");
		cleanItem.setMnemonic(KeyEvent.VK_S);
		cleanItem.setToolTipText("Wyczyœæ");
		cleanItem.addActionListener((event) -> cleanUp(frame, panel, 16, 16));
		fileMenu.add(cleanItem);

		JMenuItem exitItem = new JMenuItem("Zamknij");
		exitItem.setMnemonic(KeyEvent.VK_E);
		exitItem.setToolTipText("Zamknij program");
		exitItem.addActionListener((event) -> System.exit(0));
		fileMenu.add(exitItem);

		menuBar.add(fileMenu);

		frame.setJMenuBar(menuBar);

		currentOptionsLabel.setText("Obecnie uzywane: ");
		currentOptionsLabel.setBounds(480, 380, 120, 40);
		panel.add(currentOptionsLabel);

		currentOptionLabel.setText("\".\"");
		currentOptionLabel.setBounds(590, 380, 40, 40);
		panel.add(currentOptionLabel);

	}

	public static void createLevel(JFrame frame, JPanel panel, JLabel currentOptionLabel, int dimX, int dimY) {
		cleanUp(frame, panel, dimX, dimY);
		for (int y = 0; y < dimY; y++) {
			for (int x = 0; x < dimX; x++) {

				outputString += ".";
				JButton button = new JButton();
				button.setName(Integer.toString(dimX * y + x));
				button.setMargin(new Insets(0, 0, 0, 0));
				button.setText(".");
				button.setBounds(10 + x * 25, 10 + y * 25, 24, 24);
				button.addActionListener((event) -> {
					if ((currentMode == "O" && outputString.contains("O"))
							|| (currentMode == "X" && outputString.contains("X"))) {
						JOptionPane.showMessageDialog(frame, "Mozesz ustawic tylko jeden punkt wejscia i wyjscia!",
								"Uwaga!", JOptionPane.WARNING_MESSAGE);
						return;
					}
					button.setText(currentMode);
					char[] c = outputString.toCharArray();
					c[Integer.parseInt(button.getName())] = currentMode.charAt(0);
					outputString = new String(c);
				});
				button.updateUI();
				panel.add(button);
			}
		}
		SwingUtilities.updateComponentTreeUI(frame);
		openEditor(frame, panel, currentOptionLabel);
	}

	public static void loadMap(JFrame frame, JPanel panel, JLabel currentOptionLabel, int dimX, int dimY,
			File mapFile) {
		cleanUp(frame, panel, dimX, dimY);
		try {
			Scanner reader = new Scanner(mapFile, "utf-16");
			while (reader.hasNextLine()) {
				String s = reader.nextLine();
				if(s.charAt(0) == '/' && s.charAt(1) == '/')
					continue;
				outputString += s;
				System.out.println(s);
			}
			reader.close();
		} catch (FileNotFoundException e) {
			JOptionPane.showMessageDialog(frame, "Nie znaleziono pliku!", "Uwaga!", JOptionPane.WARNING_MESSAGE);
			e.printStackTrace();
		}

		for (int y = 0; y < dimY; y++) {
			for (int x = 0; x < dimX; x++) {
				int pos = dimX * y + x;
				JButton button = new JButton();
				button.setName(Integer.toString(pos));
				button.setMargin(new Insets(0, 0, 0, 0));
				button.setText(Character.toString(outputString.charAt(pos)));
				button.setBounds(10 + x * 25, 10 + y * 25, 24, 24);
				button.addActionListener((event) -> {
					if ((currentMode == "O" && outputString.contains("O"))
							|| (currentMode == "X" && outputString.contains("X"))) {
						JOptionPane.showMessageDialog(frame, "Mozesz ustawic tylko jeden punkt wejscia i wyjscia!",
								"Uwaga!", JOptionPane.WARNING_MESSAGE);
						return;
					}
					button.setText(currentMode);
					char[] c = outputString.toCharArray();
					c[Integer.parseInt(button.getName())] = currentMode.charAt(0);
					outputString = new String(c);
				});
				button.updateUI();
				panel.add(button);
			}
		}
		SwingUtilities.updateComponentTreeUI(frame);
		openEditor(frame, panel, currentOptionLabel);
	}

	public static void openEditor(JFrame frame, JPanel panel, JLabel currentOptionLabel) {
		JButton wallButton = new JButton();
		wallButton.setMargin(new Insets(0, 0, 0, 0));
		wallButton.setFont(new Font("Arial", 0, 20));
		wallButton.setText("#");
		wallButton.setBounds(566, 10, 48, 48);
		wallButton.addActionListener((event) -> {
			currentMode = wallButton.getText();
			currentOptionLabel.setText("\"#\"");
		});
		panel.add(wallButton);

		JButton emptySpaceButton = new JButton();
		emptySpaceButton.setMargin(new Insets(0, 0, 0, 0));
		emptySpaceButton.setFont(new Font("Arial", 0, 20));
		emptySpaceButton.setText(".");
		emptySpaceButton.setBounds(566, 60, 48, 48);
		emptySpaceButton.addActionListener((event) -> {
			currentMode = emptySpaceButton.getText();
			currentOptionLabel.setText("\".\"");
		});
		panel.add(emptySpaceButton);

		JButton playerButton = new JButton();
		playerButton.setMargin(new Insets(0, 0, 0, 0));
		playerButton.setFont(new Font("Arial", 0, 20));
		playerButton.setText("O");
		playerButton.setBounds(566, 110, 48, 48);
		playerButton.addActionListener((event) -> {
			currentMode = playerButton.getText();
			currentOptionLabel.setText("\"O\"");
		});
		panel.add(playerButton);

		JButton finishButton = new JButton();
		finishButton.setMargin(new Insets(0, 0, 0, 0));
		finishButton.setFont(new Font("Arial", 0, 20));
		finishButton.setText("X");

		finishButton.setBounds(566, 160, 48, 48);
		finishButton.addActionListener((event) -> {
			currentMode = finishButton.getText();
			currentOptionLabel.setText("\"X\"");
		});
		panel.add(finishButton);
	}

	public static void saveMap(JFrame frame, int width, int height) {
		System.out.println(outputString.length());
		if (outputString.length() != width * height)
			JOptionPane.showMessageDialog(frame, "Musisz najpierw stworzyc lub otworzyc mape!", "Uwaga!",
					JOptionPane.WARNING_MESSAGE);
		if (!outputString.contains("O") || !outputString.contains("X")) {
			JOptionPane.showMessageDialog(frame, "Twoja mapa musi posiadac przynajmniej jedno wejscie i wyjscie!",
					"Uwaga!", JOptionPane.WARNING_MESSAGE);
			return;
		}
		String oString = "";
		for (int i = 0; i < outputString.length(); i += width) {
			oString += outputString.substring(i, Math.min(outputString.length(), i + width)) + '\n';
		}

		try {
			Writer writer = new OutputStreamWriter(new FileOutputStream("map.txt"), "UTF-16");
			writer.write(oString);
			writer.close();
		} catch (Exception e) {
			e.printStackTrace();
			JOptionPane.showMessageDialog(frame, "Nie udalo sie zapisac pliku!", "Uwaga!", JOptionPane.WARNING_MESSAGE);
		}

	}

	public static void cleanUp(JFrame frame, JPanel panel, int width, int height) {
		outputString = "";
		for (Component comp : panel.getComponents()) {
			if (comp instanceof JButton) {
				panel.remove(comp);
			}
		}
		SwingUtilities.updateComponentTreeUI(frame);
	}
}
