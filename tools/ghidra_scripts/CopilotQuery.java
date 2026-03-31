//@category Copilot

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.data.DataType;
import ghidra.program.model.data.DataTypeComponent;
import ghidra.program.model.data.Enum;
import ghidra.program.model.data.Structure;
import ghidra.program.model.data.TypeDef;
import ghidra.program.model.data.Union;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.symbol.Symbol;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

public class CopilotQuery extends GhidraScript {
	private static final String MARKER = "COPILOT_JSON:";

	@Override
	public void run() throws Exception {
		String[] args = getScriptArgs();
		if (args.length < 1) {
			throw new IllegalArgumentException("CopilotQuery.java requires a mode argument");
		}

		String mode = args[0];
		if ("programs".equals(mode)) {
			runPrograms();
			return;
		}
		if ("find-function".equals(mode)) {
			runFindFunction(args);
			return;
		}
		if ("decompile".equals(mode)) {
			runDecompile(args);
			return;
		}
		if ("disasm".equals(mode)) {
			runDisasm(args);
			return;
		}
		if ("type-get".equals(mode)) {
			runTypeGet(args);
			return;
		}

		throw new IllegalArgumentException("Unsupported mode: " + mode);
	}

	private void runPrograms() {
		Map<String, Object> payload = new LinkedHashMap<String, Object>();
		payload.put("compiler_spec_id", String.valueOf(currentProgram.getCompilerSpec().getCompilerSpecID()));
		payload.put("image_base", String.valueOf(currentProgram.getImageBase()));
		payload.put("language_id", String.valueOf(currentProgram.getLanguageID()));
		payload.put("name", currentProgram.getName());
		payload.put("pathname", currentProgram.getDomainFile().getPathname());
		emit(payload);
	}

	private void runFindFunction(String[] args) {
		if (args.length < 2) {
			throw new IllegalArgumentException("find-function requires <query> [limit]");
		}
		String query = args[1].toLowerCase();
		int limit = args.length > 2 ? Integer.parseInt(args[2]) : 20;

		List<Map<String, Object>> matches = new ArrayList<Map<String, Object>>();
		FunctionIterator iterator = currentProgram.getFunctionManager().getFunctions(true);
		while (iterator.hasNext()) {
			Function function = iterator.next();
			Symbol symbol = function.getSymbol();
			String fullName = symbol != null ? symbol.getName(true) : function.getName();
			String shortName = function.getName();
			String fullLower = fullName.toLowerCase();
			String shortLower = shortName.toLowerCase();
			if (!fullLower.contains(query) && !shortLower.contains(query)) {
				continue;
			}

			int rank = 2;
			if (query.equals(fullLower) || query.equals(shortLower)) {
				rank = 0;
			}
			else if (fullLower.startsWith(query) || shortLower.startsWith(query)) {
				rank = 1;
			}

			Map<String, Object> payload = new LinkedHashMap<String, Object>();
			payload.put("_rank", Integer.valueOf(rank));
			payload.put("entry_point", String.valueOf(function.getEntryPoint()));
			payload.put("full_name", fullName);
			payload.put("name", shortName);
			payload.put("program_name", currentProgram.getName());
			payload.put("program_path", currentProgram.getDomainFile().getPathname());
			payload.put("signature", function.getPrototypeString(true, true));
			matches.add(payload);
		}

		matches.sort(new Comparator<Map<String, Object>>() {
			@Override
			public int compare(Map<String, Object> left, Map<String, Object> right) {
				int rank = ((Integer) left.get("_rank")).compareTo((Integer) right.get("_rank"));
				if (rank != 0) {
					return rank;
				}
				int address = ((String) left.get("entry_point")).compareTo((String) right.get("entry_point"));
				if (address != 0) {
					return address;
				}
				return ((String) left.get("full_name")).compareToIgnoreCase((String) right.get("full_name"));
			}
		});

		for (int i = 0; i < matches.size() && i < limit; i++) {
			Map<String, Object> payload = matches.get(i);
			payload.remove("_rank");
			emit(payload);
		}
	}

	private void runDecompile(String[] args) throws Exception {
		if (args.length < 2) {
			throw new IllegalArgumentException("decompile requires <address> [timeout]");
		}
		String addressText = args[1];
		int timeoutSeconds = args.length > 2 ? Integer.parseInt(args[2]) : 60;

		Address address = toAddr(addressText);
		if (address == null) {
			throw new IllegalArgumentException("Could not parse address: " + addressText);
		}

		Function function = currentProgram.getFunctionManager().getFunctionContaining(address);
		if (function == null) {
			function = currentProgram.getFunctionManager().getFunctionAt(address);
		}
		if (function == null) {
			throw new IllegalArgumentException("No function contains address " + addressText);
		}

		DecompInterface decompiler = new DecompInterface();
		decompiler.toggleCCode(true);
		decompiler.toggleSyntaxTree(false);
		decompiler.openProgram(currentProgram);

		DecompileResults result = decompiler.decompileFunction(function, timeoutSeconds, monitor);
		if (!result.decompileCompleted()) {
			throw new RuntimeException("Ghidra decompiler failed: " + result.getErrorMessage());
		}

		Symbol symbol = function.getSymbol();
		Map<String, Object> payload = new LinkedHashMap<String, Object>();
		payload.put("address", addressText);
		payload.put("code", result.getDecompiledFunction() != null ? result.getDecompiledFunction().getC() : "");
		payload.put("entry_point", String.valueOf(function.getEntryPoint()));
		payload.put("full_name", symbol != null ? symbol.getName(true) : function.getName());
		payload.put("name", function.getName());
		payload.put("program_name", currentProgram.getName());
		payload.put("program_path", currentProgram.getDomainFile().getPathname());
		payload.put("signature", function.getPrototypeString(true, true));
		emit(payload);
	}

	private void runDisasm(String[] args) {
		if (args.length < 2) {
			throw new IllegalArgumentException("disasm requires <address> [count]");
		}
		String addressText = args[1];
		int count = args.length > 2 ? Integer.parseInt(args[2]) : 20;

		Address address = toAddr(addressText);
		if (address == null) {
			throw new IllegalArgumentException("Could not parse address: " + addressText);
		}

		List<Map<String, Object>> instructions = new ArrayList<Map<String, Object>>();
		InstructionIterator iterator = currentProgram.getListing().getInstructions(address, true);
		while (iterator.hasNext() && instructions.size() < count) {
			Instruction instruction = iterator.next();
			Map<String, Object> item = new LinkedHashMap<String, Object>();
			item.put("address", String.valueOf(instruction.getAddress()));
			item.put("text", instruction.toString());
			instructions.add(item);
		}

		Map<String, Object> payload = new LinkedHashMap<String, Object>();
		payload.put("address", addressText);
		payload.put("instructions", instructions);
		payload.put("program_name", currentProgram.getName());
		payload.put("program_path", currentProgram.getDomainFile().getPathname());
		emit(payload);
	}

	private void runTypeGet(String[] args) {
		if (args.length < 2) {
			throw new IllegalArgumentException("type-get requires <query> [limit]");
		}
		String query = args[1].toLowerCase();
		int limit = args.length > 2 ? Integer.parseInt(args[2]) : 10;

		List<Map<String, Object>> matches = new ArrayList<Map<String, Object>>();
		Iterator<DataType> iterator = currentProgram.getDataTypeManager().getAllDataTypes();
		while (iterator.hasNext()) {
			DataType dataType = iterator.next();
			String name = dataType.getName();
			String pathName = dataType.getPathName();
			String lowerName = name.toLowerCase();
			String lowerPath = pathName.toLowerCase();
			if (!lowerName.contains(query) && !lowerPath.contains(query)) {
				continue;
			}

			int rank = 2;
			if (query.equals(lowerName) || query.equals(lowerPath)) {
				rank = 0;
			}
			else if (lowerName.startsWith(query) || lowerPath.startsWith(query)) {
				rank = 1;
			}

			Map<String, Object> payload = buildTypePayload(dataType);
			payload.put("_rank", Integer.valueOf(rank));
			matches.add(payload);
		}

		matches.sort(new Comparator<Map<String, Object>>() {
			@Override
			public int compare(Map<String, Object> left, Map<String, Object> right) {
				int rank = ((Integer) left.get("_rank")).compareTo((Integer) right.get("_rank"));
				if (rank != 0) {
					return rank;
				}
				return ((String) left.get("path_name")).compareToIgnoreCase((String) right.get("path_name"));
			}
		});

		for (int i = 0; i < matches.size() && i < limit; i++) {
			Map<String, Object> payload = matches.get(i);
			payload.remove("_rank");
			emit(payload);
		}
	}

	private Map<String, Object> buildTypePayload(DataType dataType) {
		Map<String, Object> payload = new LinkedHashMap<String, Object>();
		payload.put("category_path", dataType.getCategoryPath().getPath());
		payload.put("length", Integer.valueOf(dataType.getLength()));
		payload.put("name", dataType.getName());
		payload.put("path_name", dataType.getPathName());

		if (dataType instanceof Structure) {
			payload.put("kind", "structure");
			payload.put("components", buildComponents(((Structure) dataType).getComponents()));
		}
		else if (dataType instanceof Union) {
			payload.put("kind", "union");
			payload.put("components", buildComponents(((Union) dataType).getComponents()));
		}
		else if (dataType instanceof Enum) {
			payload.put("kind", "enum");
			Enum enumType = (Enum) dataType;
			List<Map<String, Object>> values = new ArrayList<Map<String, Object>>();
			for (String enumName : enumType.getNames()) {
				Map<String, Object> item = new LinkedHashMap<String, Object>();
				item.put("name", enumName);
				item.put("value", Long.valueOf(enumType.getValue(enumName)));
				values.add(item);
			}
			payload.put("values", values);
		}
		else if (dataType instanceof TypeDef) {
			payload.put("kind", "typedef");
			payload.put("base_type", ((TypeDef) dataType).getDataType().getDisplayName());
		}
		else {
			payload.put("kind", dataType.getClass().getSimpleName());
		}

		return payload;
	}

	private List<Map<String, Object>> buildComponents(DataTypeComponent[] components) {
		List<Map<String, Object>> items = new ArrayList<Map<String, Object>>();
		for (DataTypeComponent component : components) {
			Map<String, Object> payload = new LinkedHashMap<String, Object>();
			payload.put("comment", component.getComment() != null ? component.getComment() : "");
			payload.put("field_name", component.getFieldName() != null ? component.getFieldName() : "");
			payload.put("length", Integer.valueOf(component.getLength()));
			payload.put("offset", Integer.valueOf(component.getOffset()));
			payload.put("type_name", component.getDataType().getDisplayName());
			items.add(payload);
		}
		return items;
	}

	private void emit(Map<String, Object> payload) {
		println(MARKER + toJson(payload));
	}

	private String toJson(Object value) {
		if (value == null) {
			return "null";
		}
		if (value instanceof String) {
			return quote((String) value);
		}
		if (value instanceof Number || value instanceof Boolean) {
			return String.valueOf(value);
		}
		if (value instanceof Map) {
			StringBuilder out = new StringBuilder();
			out.append('{');
			boolean first = true;
			for (Object entryObject : ((Map<?, ?>) value).entrySet()) {
				Map.Entry<?, ?> entry = (Map.Entry<?, ?>) entryObject;
				if (!first) {
					out.append(',');
				}
				first = false;
				out.append(quote(String.valueOf(entry.getKey())));
				out.append(':');
				out.append(toJson(entry.getValue()));
			}
			out.append('}');
			return out.toString();
		}
		if (value instanceof Iterable) {
			StringBuilder out = new StringBuilder();
			out.append('[');
			boolean first = true;
			for (Object item : (Iterable<?>) value) {
				if (!first) {
					out.append(',');
				}
				first = false;
				out.append(toJson(item));
			}
			out.append(']');
			return out.toString();
		}
		return quote(String.valueOf(value));
	}

	private String quote(String text) {
		StringBuilder out = new StringBuilder();
		out.append('"');
		for (int i = 0; i < text.length(); i++) {
			char ch = text.charAt(i);
			switch (ch) {
				case '\\':
					out.append("\\\\");
					break;
				case '"':
					out.append("\\\"");
					break;
				case '\n':
					out.append("\\n");
					break;
				case '\r':
					out.append("\\r");
					break;
				case '\t':
					out.append("\\t");
					break;
				default:
					if (ch < 0x20) {
						out.append(String.format("\\u%04x", Integer.valueOf(ch)));
					}
					else {
						out.append(ch);
					}
					break;
			}
		}
		out.append('"');
		return out.toString();
	}
}
