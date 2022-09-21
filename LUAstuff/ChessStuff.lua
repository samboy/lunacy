#!/usr/bin/env lua

-- Chess stuff implemented in pure Lua/Lunacy

-- Public domain -or- BSD0 by Sam Trenholme.  See LICENSE file.  If
-- a LICENSE file is not provided, the files here are public domain.

-- Convert an algebraic square ('a1', etc.) in to a grid coordinate
-- The grid has a8 be '1' and h1 be 64 so that FEN can be directly 
-- translated in to the grid.  It is one-dimensional
function algToGrid(alg) 
  local fileS = {a=1,b=2,c=3,d=4,e=5,f=6,g=7,h=8}
  local file = alg:sub(1,1)
  local rank = tonumber(alg:sub(2,2))
  return 8*(8-rank) + fileS[file]
end

-- Convert a move to the grid points for both the source and destination
-- Input is something like 'e2e4'
function algToGrid2(alg)
  local from = algToGrid(alg:sub(1,2))
  local to = algToGrid(alg:sub(3,4))
  return from, to
end

-- Convert an algrbraic square in to an x,y coordinate
-- Input: String in form 'e4'
function algToXY(alg)
  local fileS = {a=1,b=2,c=3,d=4,e=5,f=6,g=7,h=8}
  local file = alg:sub(1,1)
  file = fileS[file] -- Make number
  local rank = tonumber(alg:sub(2,2))
  return tonumber(file), tonumber(rank)
end

-- Determine the x,y delta in a piece move
-- Input: String in form 'e2e4' or 'e7e8q'
function determineDelta(alg)
  local from = alg:sub(1,2)
  local to = alg:sub(3,4)
  local fromX, fromY = algToXY(from)
  local toX, toY = algToXY(to)
  local deltaX = math.abs(fromX - toX)
  local deltaY = math.abs(fromY - toY)
  return deltaX, deltaY
end

-- Could this move be a pawn capture
-- Input: String in form 'e2e4' or 'e7e8q'
function pawnCaptureMove(alg)
  local x, y = determineDelta(alg)
  if x == 1 and y == 1 then return true end
  return false
end

-- Could this move be a normal king move
-- Input: String in form 'e2e4' or 'e7e8q'
function normalKingMove(alg)
  local x, y = determineDelta(alg)
  if x <= 1 and y <= 1 then return true end
  return false
end

-- Determine if a given move is an unusual move (promotion, en passent,
-- castling)
-- Input is a move like 'e2e4' and the state of the chessboard ('grid')
-- before the move is made
function nonStandardMove(alg, grid)
  if alg:len() ~= 4 then return true end -- Promotion
  local from, to = algToGrid2(alg)
  if grid[from]:lower() == 'p' and pawnCaptureMove(alg) and 
     grid[to]:lower() == '' then return true end -- En passant
  if grid[from]:lower() == 'k' and not normalKingMove(alg) then 
    return true -- Castling
  end
  return false
end

-- If we have a standard move (no promotion, en passant, or castling), 
-- update the grid (chessboard) with the move made
-- Input: Move like 'e2e4', grid (chessboard) with current state
-- Note: Grid will be altered by this function!
function movePiece(alg, grid)
  if nonStandardMove(alg, grid) then return false end -- Not implemented
  local from, to = algToGrid2(alg)
  grid[to] = grid[from]
  grid[from] = ""
  return true
end

-- New empty chessboard
function emptyChessboard()
  local out = {}
  for a=1,64 do table.insert(out,"") end
  return out
end

-- Convert a FEN in to a grid
function FENtoGrid(fen)
  local out = emptyChessboard()
  local number = 0
  local index = 1
  local square = 1
  while index < fen:len() do
    local look = fen:sub(index, index)
    index = index + 1
    if look:match('[0-9]') then
      number = number * 10
      number = number + tonumber(look)
    else
      square = square + number
      number = 0
    end
    if look:match('[a-zA-Z]') then
      out[square] = look
      square = square + 1
    end
    if look:match('%s') then
      return out
    end
  end
  return out
end

-- Convert a FEN piece in to its Chess Merida character
function pieceToMerida(piece, isBlackSquare)
  local isBlack, isPiece, thisPiece
  isPiece = piece:match('[a-zA-Z]')
  isBlack = piece:match('[a-z]')
  if not isPiece then
    if isBlackSquare then return "+" else return " " end
  end
  thisPiece = piece:lower()
  if isBlack then
    local blackMap = {q="w",r="t",p="o",k="l",b="v",n="m"}
    thisPiece = blackMap[thisPiece]
  end
  if isBlackSquare then thisPiece = thisPiece:upper() end
  return thisPiece
end

-- The color of a given square on a chessboard
-- Index is a number from 1 (a8) to 64 (h1)
-- Output is true if square is black, false otherwise
function squareIsBlack(index)
  local phase = math.floor((index - 1) / 8) % 2
  if phase == 1 then
    if(index % 2) == 1 then return true else return false end
  else
    if(index % 2) == 0 then return true else return false end
  end
end

-- Convert a Grid in to a chessboard rendered by Chess Merida font
function gridToMerida(grid, newline)
  if not newline then newline = "\n" end
  local out = '!""""""""#' .. newline
  local index = 1
  for row = 1,8 do
    out = out .. "$"
    for file = 1,8 do
      out = out .. pieceToMerida(grid[index],squareIsBlack(index))
      index = index + 1
    end
    out = out .. "%" .. newline
  end
  out = out .. "/(((((((()" .. newline
  return out
end

function FENtoMerida(fen)
  local grid = FENtoGrid(fen)
  return gridToMerida(grid)
end

-- Since Lunacy doesn't have split(), we make
-- it ourselves
function chessSplit(s, splitOn)
  if not splitOn then splitOn = "," end
  local place = true
  local out = {}
  local mark
  local last = 1
  while place do
    place, mark = string.find(s, splitOn, last, false)
    if place then
      table.insert(out,string.sub(s, last, place - 1))
      last = mark + 1
    end
  end
  table.insert(out,string.sub(s, last, -1))
  return out
end

-- Given a single mate In 1 puzzle from https://database.lichess.org, 
-- if the move in to the mate-in-1 isn’t unusual (promotion, en passant, or
-- castling), output a Chess Merida font compatible version of the puzzle
-- for easier printing
function lichessToMerida(line)
  local fields = chessSplit(line)
  local fen = fields[2]
  local moves = fields[3]
  local rating = tonumber(fields[4])
  local grid = FENtoGrid(fen)
  local moveList = chessSplit(moves,' ')
  if movePiece(moveList[1],grid) then return gridToMerida(grid) end
  return false
end

